// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSNetworking/Socket/SocketSubsystemRedpointEOS.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSAPI/P2P/GetNextReceivedPacketSize.h"
#include "RedpointEOSAPI/P2P/GetPacketQueueInfo.h"
#include "RedpointEOSAPI/P2P/NotifyIncomingPacketQueueFull.h"
#include "RedpointEOSAPI/P2P/ReceivePacket.h"
#include "RedpointEOSAPI/P2P/SetPacketQueueSize.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSCore/Utils/MakeSharedWithEvents.h"
#include "RedpointEOSIdentity/IdentitySystem.h"
#include "RedpointEOSNetworking/Logging.h"
#include "RedpointEOSNetworking/Socket/ListenManager.h"
#include "RedpointEOSNetworking/Socket/P2PChannel.h"
#include "RedpointEOSNetworking/Socket/PacketTiming.h"
#include "RedpointEOSNetworking/Socket/ProtocolType.h"
#include "RedpointEOSNetworking/Socket/SocketConnectionMapKey.h"
#include "RedpointEOSNetworking/Socket/SocketRole.h"
#include "RedpointEOSNetworking/Socket/SocketRoleClient.h"
#include "RedpointEOSNetworking/Socket/SocketRoleListening.h"
#include "RedpointEOSNetworking/Socket/SocketRoleNone.h"
#include "RedpointEOSNetworking/Socket/SocketRoleRemote.h"
#include "RedpointEOSNetworking/Socket/SocketRoleRemoteState.h"
#include "RedpointEOSNetworking/Socket/SocketTracing.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(2367027670, Redpoint::EOS::Networking::Socket)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::Core::Utils;
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::Networking;
using namespace ::Redpoint::EOS::Networking::Socket;

FSocketRedpointEOSMemoryId FSocketSubsystemRedpointEOS::NextSocketId = 1000;

FName FSocketSubsystemRedpointEOS::GetSystemName()
{
    return FName(TEXT("SocketSubsystemRedpointEOS"));
}

TSharedRef<FSocketSubsystemRedpointEOS> FSocketSubsystemRedpointEOS::ConstructSystem(
    FPlatformHandle InPlatformHandle,
    FPlatformInstanceEnvironmentRef InPlatformInstanceEnvironment)
{
    auto SocketSubsystem =
        MakeShared<FSocketSubsystemRedpointEOS>(InPlatformHandle, InPlatformInstanceEnvironment->Config);

    // @note: This can't actually fail, but Init() is required to have this signature to implement ISocketSubsystem.
    FString Error;
    SocketSubsystem->Init(Error);

    InPlatformHandle->OnShutdown().AddSP(SocketSubsystem, &FSocketSubsystemRedpointEOS::OnShutdown);

    return SocketSubsystem;
}

void FSocketSubsystemRedpointEOS::OnShutdown(FPlatformInstance &Instance)
{
    this->Shutdown();
}

FSocketSubsystemRedpointEOS::FSocketSubsystemRedpointEOS(
    const FPlatformHandle &InPlatform,
    TSharedRef<Redpoint::EOS::Config::IConfig> InConfig)
    : PlatformHandle(InPlatform)
    , bIsShutdown(false)
    , bReceiveSuspended(false)
    , Config(MoveTemp(InConfig))
    , ReceivePacketsDelegateHandle()
    , HeldSockets()
    , AssignedSockets()
    , RoleInstance_None(MakeShared<FSocketRoleNone>())
    , RoleInstance_Client(MakeShared<FSocketRoleClient>())
    , RoleInstance_Listening(MakeShared<FSocketRoleListening>())
    , RoleInstance_Remote(MakeShared<FSocketRoleRemote>())
    , ListenManager(MakeSharedWithEvents<FListenManager>(InPlatform))
    // If the game were to crash and this was simply initialized to 0 every time, then when the player attempts to
    // reconnect to the server they were playing on, it would result in a reset ID of 0 being sent. If the server hasn't
    // yet cleaned up the connection (due to unclean shutdown), then it would not be aware that it needs to reset the
    // connection. By setting this value to a timestamp, we mitigate this problem. In addition we don't care about
    // overflows here because the reset ID is only ever compared exactly, and is not used in relative comparisons.
    , NextResetId((uint32_t)FDateTime::UtcNow().ToUnixTimestamp())
    , PendingResetIds()
    , IncomingPacketQueueFull()
{
}

FString FSocketSubsystemRedpointEOS::GetSocketName(bool bListening, FURL InURL) const
{
    if (bListening)
    {
        // If we are listening, get it from the option.
        auto SocketName = FString(InURL.GetOption(TEXT("SocketName="), TEXT("default")));
        if (SocketName.Len() > P2PSocketNameMaxLength)
        {
            return SocketName.Mid(0, P2PSocketNameMaxLength);
        }
        else
        {
            return SocketName;
        }
    }
    else
    {
        // Otherwise get it from the URL hostname.
        auto Addr = MakeShared<FInternetAddrRedpointEOS>();
        bool bIsValid = false;
        Addr->SetIp(*InURL.Host, bIsValid);
        if (!bIsValid)
        {
            UE_LOG(
                LogRedpointEOSNetworking,
                Warning,
                TEXT("Unable to parse address as EOS address to determine socket name '%s', defaulting to 'default'."),
                *InURL.Host);
            return FString(TEXT("default"));
        }
        else
        {
            return Addr->GetSymmetricSocketId();
        }
    }
}

bool FSocketSubsystemRedpointEOS::Init(FString &Error)
{
    Error = TEXT("");

    // Show default P2P queue size.
    FGetPacketQueueInfo::Result QueueInfo = {};
    auto QueueResult = FGetPacketQueueInfo::Execute(this->PlatformHandle, FGetPacketQueueInfo::Options{}, QueueInfo);
    if (QueueResult == EOS_EResult::EOS_Success)
    {
        UE_LOG(
            LogRedpointEOSNetworking,
            Verbose,
            TEXT("EOS P2P packet queue info: incoming_max_size=%llu, incoming_current_size=%llu, "
                 "incoming_packet_count=%llu, outgoing_max_size=%llu, outgoing_current_size=%llu, "
                 "outgoing_packet_count=%llu"),
            QueueInfo.IncomingPacketQueueMaxSizeBytes,
            QueueInfo.IncomingPacketQueueCurrentSizeBytes,
            QueueInfo.IncomingPacketQueueCurrentPacketCount,
            QueueInfo.OutgoingPacketQueueMaxSizeBytes,
            QueueInfo.OutgoingPacketQueueCurrentSizeBytes,
            QueueInfo.OutgoingPacketQueueCurrentPacketCount);
    }

    // Set P2P to have unlimited packet queue sizes.
    {
        auto SizeResult = FSetPacketQueueSize::Execute(
            this->PlatformHandle,
            FSetPacketQueueSize::Options{
                .IncomingPacketQueueMaxSizeBytes = static_cast<uint64>(EOS_P2P_MAX_QUEUE_SIZE_UNLIMITED),
                .OutgoingPacketQueueMaxSizeBytes = static_cast<uint64>(EOS_P2P_MAX_QUEUE_SIZE_UNLIMITED)});
        if (SizeResult != EOS_EResult::EOS_Success)
        {
            UE_LOG(
                LogRedpointEOSNetworking,
                Warning,
                TEXT("%s"),
                *ConvertError(
                     ANSI_TO_TCHAR(__FUNCTION__),
                     TEXT("Unable to set P2P packet queue size to unlimited."),
                     SizeResult)
                     .ToLogString());
        }
    }

    // Show new P2P queue size after adjustment.
    QueueResult = FGetPacketQueueInfo::Execute(this->PlatformHandle, FGetPacketQueueInfo::Options{}, QueueInfo);
    if (QueueResult == EOS_EResult::EOS_Success)
    {
        UE_LOG(
            LogRedpointEOSNetworking,
            Verbose,
            TEXT("EOS P2P packet queue info: incoming_max_size=%llu, incoming_current_size=%llu, "
                 "incoming_packet_count=%llu, outgoing_max_size=%llu, outgoing_current_size=%llu, "
                 "outgoing_packet_count=%llu"),
            QueueInfo.IncomingPacketQueueMaxSizeBytes,
            QueueInfo.IncomingPacketQueueCurrentSizeBytes,
            QueueInfo.IncomingPacketQueueCurrentPacketCount,
            QueueInfo.OutgoingPacketQueueMaxSizeBytes,
            QueueInfo.OutgoingPacketQueueCurrentSizeBytes,
            QueueInfo.OutgoingPacketQueueCurrentPacketCount);
    }

    // Listen for notifications that the packet queue is full.
    this->IncomingPacketQueueFull = FNotifyIncomingPacketQueueFull::Register(
        this->PlatformHandle,
        FNotifyIncomingPacketQueueFull::Options{},
        FNotifyIncomingPacketQueueFull::EventDelegate::CreateSPLambda(
            this,
            [this](const FNotifyIncomingPacketQueueFull::Result &Info) {
                uint64 NewPacketQueueSize = Info.PacketQueueMaxSizeBytes + ((uint64)Info.OverflowPacketSizeBytes * 2);
                UE_LOG(
                    LogRedpointEOSNetworking,
                    Warning,
                    TEXT("Incoming packet queue has become full! Max size = %llu, current size = %llu, overflow "
                         "size = %llu, new size = %llu."),
                    Info.PacketQueueMaxSizeBytes,
                    Info.PacketQueueCurrentSizeBytes,
                    Info.OverflowPacketSizeBytes,
                    NewPacketQueueSize);

                auto SizeResult = FSetPacketQueueSize::Execute(
                    this->PlatformHandle,
                    FSetPacketQueueSize::Options{
                        .IncomingPacketQueueMaxSizeBytes = NewPacketQueueSize,
                        .OutgoingPacketQueueMaxSizeBytes = static_cast<uint64>(EOS_P2P_MAX_QUEUE_SIZE_UNLIMITED)});
                if (SizeResult != EOS_EResult::EOS_Success)
                {
                    UE_LOG(
                        LogRedpointEOSNetworking,
                        Error,
                        TEXT("%s"),
                        *ConvertError(
                             ANSI_TO_TCHAR(__FUNCTION__),
                             TEXT("Unable to set P2P packet queue size in response to overflow! Packets will be lost!"),
                             SizeResult)
                             .ToLogString());
                }
            }));

    // Start ticking to receive P2P packets.
    this->ReceivePacketsDelegateHandle = FRegulatedTicker::GetCoreTicker().AddTicker(
        FTickerDelegate::CreateSP(this->AsShared(), &FSocketSubsystemRedpointEOS::Tick_ReceiveP2PPackets));

    return true;
}

void FSocketSubsystemRedpointEOS::Shutdown()
{
    // Make sure we only shutdown once.
    if (this->bIsShutdown)
    {
        return;
    }
    this->bIsShutdown = true;

    // Deregister the packet queue handler.
    this->IncomingPacketQueueFull.Reset();

    // Stop the packet receive ticker.
    FRegulatedTicker::GetCoreTicker().RemoveTicker(this->ReceivePacketsDelegateHandle);
    this->ReceivePacketsDelegateHandle.Reset();

    // Remove all sockets from the assignment map, since we are about to free them.
    this->AssignedSockets.Empty();

    // Free all sockets in memory.
    TArray<int> Keys;
    this->HeldSockets.GetKeys(Keys);
    for (int Key : Keys)
    {
        // Because DestroySocket also destroys children, and those children
        // will be removed from the HeldSockets array after we've captured
        // the keys..
        if (this->HeldSockets.Contains(Key))
        {
            FSocketRedpointEOS *SocketEOS = this->HeldSockets[Key].Get();
            this->DestroySocket(SocketEOS);
        }
    }
    check(this->HeldSockets.Num() == 0);
}

bool FSocketSubsystemRedpointEOS::Tick_ReceiveP2PPackets(float DeltaSeconds)
{
    if (this->bReceiveSuspended)
    {
        return true;
    }

    auto IdentitySystem = this->PlatformHandle->GetSystem<IIdentitySystem>();
    for (const auto &User : IdentitySystem->GetUsers())
    {
        auto DestinationAccountId = User->GetUserId();
        if (!IsProductUserId(DestinationAccountId) && IsDedicatedServerId(DestinationAccountId))
        {
            continue;
        }

        auto DestinationUserId = GetProductUserId(DestinationAccountId);

        while (true)
        {
            FGetNextReceivedPacketSize::Result PendingDataSize;
            if (FGetNextReceivedPacketSize::Execute(
                    this->PlatformHandle,
                    FGetNextReceivedPacketSize::Options{
                        .LocalUserId = DestinationUserId,
                        .RequestedChannel = TOptional<uint8>(),
                    },
                    PendingDataSize) != EOS_EResult::EOS_Success)
            {
                break;
            }

            void *Buffer = FMemory::Malloc(PendingDataSize);

            EOS_ProductUserId SourceUserId = {};
            FSocketId SymmetricSocketId = {};
            uint8 SymmetricChannel = 0;
            uint32 BytesRead = 0;

            auto ReceiveResult = FReceivePacket::Execute(
                this->PlatformHandle,
                FReceivePacket::Options{
                    .LocalUserId = DestinationUserId,
                    .MaxDataSizeBytes = PendingDataSize,
                    .RequestedChannel = TOptional<uint8>(),
                },
                SourceUserId,
                SymmetricSocketId,
                SymmetricChannel,
                Buffer,
                BytesRead);
            if (ReceiveResult != EOS_EResult::EOS_Success)
            {
                UE_LOG(
                    LogRedpointEOSNetworking,
                    Error,
                    TEXT("%s"),
                    *ConvertError(
                         ANSI_TO_TCHAR(__FUNCTION__),
                         TEXT("Failed to receive packet after EOS_P2P_GetNextReceivedPacketSize returned successful."),
                         ReceiveResult)
                         .ToLogString());
                FMemory::Free(Buffer);
                continue;
            }

            TSharedPtr<FPacketTiming> TimingData;
#if !(defined(UE_BUILD_SHIPPING) && UE_BUILD_SHIPPING)
            if (CVarEnablePacketTiming.GetValueOnGameThread())
            {
                if (BytesRead > sizeof(int64))
                {
                    int64 SentTicks = *((int64 *)Buffer);
                    void *NewBuffer = FMemory::Malloc(BytesRead - sizeof(int64));
                    FMemory::Memcpy(NewBuffer, (uint8 *)Buffer + sizeof(int64), BytesRead - sizeof(int64));
                    FMemory::Free(Buffer);
                    Buffer = NewBuffer;
                    BytesRead -= sizeof(int64);

                    TimingData = MakeShared<FPacketTiming>(
                        SourceUserId,
                        DestinationUserId,
                        SymmetricSocketId,
                        SymmetricChannel,
                        SentTicks,
                        FDateTime::UtcNow(),
                        BytesRead);
                }
            }
#endif

            LogSocketPacket(FSendPacket::Options{
                .LocalUserId = SourceUserId,
                .RemoteUserId = DestinationUserId,
                .SocketId = SymmetricSocketId,
                .Channel = SymmetricChannel,
                .DataLengthBytes = BytesRead,
                .Data = Buffer,
                // These options aren't used in the logging.
                .bAllowDelayedDelivery = false,
                .Reliability = EOS_EPacketReliability::EOS_PR_UnreliableUnordered,
                .bDisableAutoAcceptConnection = false,
            });

            if (SymmetricChannel == P2PChannelIdControl)
            {
                // This is a reset packet, designed so we can support re-connections on one channel while another
                // channel remains open.

                auto *BufferAsUint8 = reinterpret_cast<uint8 *>(Buffer);

                FSocketConnectionMapKey SocketKey(
                    DestinationAccountId,
                    GetAccountId(SourceUserId),
                    SymmetricSocketId,
                    BufferAsUint8[0]);

                auto *BufferAsUint32WithOffset = reinterpret_cast<uint32 *>(&BufferAsUint8[1]);

                auto ResetId = BufferAsUint32WithOffset[0];
                bool bStoreResetId = true;

                if (this->AssignedSockets.Contains(SocketKey))
                {
                    if (auto SocketToReset = this->AssignedSockets[SocketKey].Pin())
                    {
                        auto State = StaticCastSharedRef<FSocketRoleRemoteState>(SocketToReset->RoleState);

                        if (State->SocketAssignedResetId == 0)
                        {
                            // This is the reset ID for a socket that was opened due to a data packet. We now need to
                            // assign it to socket instead of closing it.
                            UE_LOG(
                                LogRedpointEOSSocket,
                                Verbose,
                                TEXT("%s: Received reset packet ID %u, matching unassigned socket. Assigning reset ID "
                                     "to socket."),
                                *SocketKey.ToString(),
                                ResetId);
                            State->SocketAssignedResetId = ResetId;
                            bStoreResetId = false;
                        }
                        else if (State->SocketAssignedResetId == ResetId)
                        {
                            // This packet must be a duplicate? Odd.
                            UE_LOG(
                                LogRedpointEOSSocket,
                                Verbose,
                                TEXT("%s: Received reset packet ID %u, identical to socket that is already open, "
                                     "ignoring."),
                                *SocketKey.ToString(),
                                ResetId);
                            bStoreResetId = false;
                        }
                        else
                        {
                            // This reset packet doesn't match the existing socket, thus we must reset the socket. Close
                            // the existing socket first.
                            UE_LOG(
                                LogRedpointEOSSocket,
                                Verbose,
                                TEXT("%s: Received reset packet ID %u, closing socket (Socket should re-open itself as "
                                     "a new connection shortly)."),
                                *SocketKey.ToString(),
                                ResetId);
                            SocketToReset->Close();

                            // If the reset ID is not 0, store the pending reset ID for the next opened socket.
                            bStoreResetId = ResetId != 0;
                        }
                    }
                }
                else
                {
                    UE_LOG(
                        LogRedpointEOSSocket,
                        Verbose,
                        TEXT("%s: Received reset packet ID %u, but no socket exists yet. Storing for when the socket "
                             "is opened later."),
                        *SocketKey.ToString(),
                        ResetId);
                }

                // Store the reset ID if needed (if there was no socket already, or if the existing socket was closed).
                if (bStoreResetId)
                {
                    auto RemoteAddr = MakeShared<FInternetAddrRedpointEOS>(
                        GetAccountId(SourceUserId),
                        SymmetricSocketId,
                        BufferAsUint8[0]);
                    this->PendingResetIds.Add(RemoteAddr->ToString(true), ResetId);
                }

#if !(defined(UE_BUILD_SHIPPING) && UE_BUILD_SHIPPING)
                RecordPacketTiming(TimingData);
#endif
                FMemory::Free(Buffer);
            }
            else
            {
                FSocketConnectionMapKey SocketKey(
                    DestinationAccountId,
                    GetAccountId(SourceUserId),
                    SymmetricSocketId,
                    SymmetricChannel);

                if (this->AssignedSockets.Contains(SocketKey))
                {
                    if (auto Socket = this->AssignedSockets[SocketKey].Pin())
                    {
                        UE_LOG(LogRedpointEOSSocket, VeryVerbose, TEXT("%s: Received packet."), *SocketKey.ToString());

                        Socket->ReceivedPacketsQueueCount++;
                        Socket->ReceivedPacketsQueue.Enqueue(MakeShared<FRawPacket>(Buffer, BytesRead, TimingData));
                    }
                    else
                    {
                        UE_LOG(
                            LogRedpointEOSNetworking,
                            Error,
                            TEXT("AssignedSockets contains a stale socket '%s', unable to route traffic."),
                            *SocketKey.ToString());
#if !(defined(UE_BUILD_SHIPPING) && UE_BUILD_SHIPPING)
                        RecordPacketTiming(TimingData);
#endif
                        FMemory::Free(Buffer);
                    }
                }
                else
                {
                    // This packet might be a new connection for a listening socket.
                    TSharedPtr<FSocketRedpointEOS> ListeningSocket = nullptr;
                    if (this->ListenManager->GetListeningSocketForNewInboundConnection(SocketKey, ListeningSocket))
                    {
                        UE_LOG(
                            LogRedpointEOSSocket,
                            Verbose,
                            TEXT("%s: Registering new socket for inbound connection."),
                            *SocketKey.ToString());
                        TSharedPtr<FInternetAddrRedpointEOS> SourceAddr = MakeShared<FInternetAddrRedpointEOS>();
                        SourceAddr->SetFromParameters(GetAccountId(SourceUserId), SymmetricSocketId, SymmetricChannel);
                        auto *AcceptedSocket = static_cast<FSocketRedpointEOS *>(
                            ListeningSocket->Accept(*SourceAddr, TEXT("EOS socket (remote)")));
                        check(AcceptedSocket != nullptr);
                        AcceptedSocket->ReceivedPacketsQueueCount++;
                        AcceptedSocket->ReceivedPacketsQueue.Enqueue(
                            MakeShared<FRawPacket>(Buffer, BytesRead, TimingData));
                    }
                    else
                    {
                        if (GIsAutomationTesting)
                        {
                            UE_LOG(
                                LogRedpointEOSNetworking,
                                Verbose,
                                TEXT("Packet arrived for intended for socket '%s', but no socket exists to receive the "
                                     "packet. Discarding."),
                                *SocketKey.ToString());
                        }
                        else
                        {
                            UE_LOG(
                                LogRedpointEOSNetworking,
                                Error,
                                TEXT("Packet arrived for intended for socket '%s', but no socket exists to receive the "
                                     "packet. Discarding."),
                                *SocketKey.ToString());
                        }
#if !(defined(UE_BUILD_SHIPPING) && UE_BUILD_SHIPPING)
                        RecordPacketTiming(TimingData);
#endif
                        FMemory::Free(Buffer);

                        // Send a reset packet to the remote to tell them that the socket was closed on our end
                        // previously.
                        {
                            FSocketId SocketId = SocketKey.GetSymmetricSocketId();

                            uint8 ChannelToResetAndId[5] = {
                                SocketKey.GetSymmetricChannel(),
                                0,
                                0,
                                0,
                                0,
                            };

                            auto Opts = FSendPacket::Options{
                                DestinationUserId,
                                SourceUserId,
                                SocketId,
                                P2PChannelIdControl,
                                5,
                                ChannelToResetAndId,
                                true,
                                EOS_EPacketReliability::EOS_PR_ReliableOrdered,
                                false};

                            LogSocketPacket(Opts);

                            auto SendResult = FSendPacket::Execute(this->PlatformHandle, Opts);

                            if (SendResult == EOS_EResult::EOS_Success)
                            {
                                UE_LOG(
                                    LogRedpointEOSNetworking,
                                    Warning,
                                    TEXT("Notified remote that sent unroutable packet that the packets are not "
                                         "routable."));
                            }
                        }

                        continue;
                    }
                }
            }
        }
    }

    return true;
}

TOptional<UE::Online::FAccountId> FSocketSubsystemRedpointEOS::GetBindingProductUserId_P2POnly() const
{
    auto IdentitySystem = this->PlatformHandle->GetSystem<IIdentitySystem>();
    auto Users = IdentitySystem->GetUsers();
    if (Users.Num() == 0)
    {
        UE_LOG(
            LogRedpointEOSNetworking,
            Verbose,
            TEXT("%s: No local user is authenticated with EOS, so there is no local user ID to use for the address"),
            ANSI_TO_TCHAR(__FUNCTION__));
        return TOptional<UE::Online::FAccountId>();
    }

    auto FirstAccount = Users[0];
    auto FirstPlayerId = FirstAccount->GetUserId();

    if (IsDedicatedServerId(FirstPlayerId))
    {
        // This is a dedicated server and the product user ID will be empty (but present for API calls).
        // Return nullptr so we don't try to use EOS P2P.
        return TOptional<UE::Online::FAccountId>();
    }

    return FirstPlayerId;
}

FSocket *FSocketSubsystemRedpointEOS::CreateSocket(
    const FName &SocketType,
    const FString &SocketDescription,
    const FName &ProtocolName)
{
    auto SocketId = FSocketSubsystemRedpointEOS::NextSocketId++;
    auto SocketEOS =
        MakeShared<FSocketRedpointEOS>(this->AsShared(), this->PlatformHandle, SocketId, SocketDescription);
    HeldSockets.Add(SocketId, SocketEOS);
    return &SocketEOS.Get();
}

void FSocketSubsystemRedpointEOS::DestroySocket(FSocket *Socket)
{
    check(Socket != nullptr);
    check(Socket->GetProtocol() == ProtocolType);

    auto *SocketEOS = static_cast<FSocketRedpointEOS *>(Socket);
    SocketEOS->Close();

    auto SocketEOSPtr = SocketEOS->AsSharedRedpointEOS();
    HeldSockets.Remove(SocketEOSPtr->GetSocketMemoryId());

    auto ChildSocketPtrs = SocketEOS->Role->GetOwnedSockets(*SocketEOSPtr);

    while (ChildSocketPtrs.Num() > 0)
    {
        if (auto FirstChildSocket = ChildSocketPtrs[0].Pin())
        {
            ChildSocketPtrs.RemoveAt(0);
            auto *ChildSocketRaw = FirstChildSocket.Get();
            FirstChildSocket.Reset();
            this->DestroySocket(ChildSocketRaw);
        }
    }

    check(SocketEOSPtr.IsUnique());
}

FAddressInfoResult FSocketSubsystemRedpointEOS::GetAddressInfo(
    const TCHAR *HostName,
    const TCHAR *ServiceName,
    EAddressInfoFlags QueryFlags,
    const FName ProtocolTypeName,
    ESocketType SocketType)
{
    return ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)
        ->GetAddressInfo(HostName, ServiceName, QueryFlags, ProtocolTypeName, SocketType);
}

TSharedPtr<FInternetAddr> FSocketSubsystemRedpointEOS::GetAddressFromString(const FString &InAddress)
{
    TSharedPtr<FInternetAddrRedpointEOS> Addr = MakeShared<FInternetAddrRedpointEOS>();
    bool bIsValid;
    Addr->SetIp(*InAddress, bIsValid);
    if (bIsValid)
    {
        return Addr;
    }

    UE_LOG(
        LogRedpointEOSNetworking,
        Error,
        TEXT("%s asked to generate address from string '%s', but it wasn't valid. This should never happen, as the EOS "
             "socket subsystem should not be used outside UEOSNetDriver/UEOSNetConnection."),
        ANSI_TO_TCHAR(__FUNCTION__),
        *InAddress);
    return nullptr;
}

bool FSocketSubsystemRedpointEOS::RequiresChatDataBeSeparate()
{
    return false;
}

bool FSocketSubsystemRedpointEOS::RequiresEncryptedPackets()
{
    return false;
}

bool FSocketSubsystemRedpointEOS::GetHostName(FString &HostName)
{
    return false;
}

TSharedRef<FInternetAddr> FSocketSubsystemRedpointEOS::CreateInternetAddr()
{
    return MakeShared<FInternetAddrRedpointEOS>();
}

bool FSocketSubsystemRedpointEOS::HasNetworkDevice()
{
    return true;
}

const TCHAR *FSocketSubsystemRedpointEOS::GetSocketAPIName() const
{
    return TEXT("EOS");
}

ESocketErrors FSocketSubsystemRedpointEOS::GetLastErrorCode()
{
    return ESocketErrors::SE_NO_ERROR;
}

ESocketErrors FSocketSubsystemRedpointEOS::TranslateErrorCode(int32 Code)
{
    return ESocketErrors::SE_NO_ERROR;
}

bool FSocketSubsystemRedpointEOS::IsSocketWaitSupported() const
{
    return false;
}

void FSocketSubsystemRedpointEOS::SuspendReceive()
{
    this->bReceiveSuspended = true;
}

void FSocketSubsystemRedpointEOS::ResumeReceive()
{
    this->bReceiveSuspended = false;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()