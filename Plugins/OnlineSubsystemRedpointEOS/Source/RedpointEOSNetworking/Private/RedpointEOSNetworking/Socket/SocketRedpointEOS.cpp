// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSNetworking/Socket/SocketRedpointEOS.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSAPI/P2P/CloseConnection.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSNetworking/Logging.h"
#include "RedpointEOSNetworking/Socket/ConsoleVariables.h"
#include "RedpointEOSNetworking/Socket/ProtocolType.h"
#include "RedpointEOSNetworking/Socket/SocketRoleNone.h"
#include "RedpointEOSNetworking/Socket/SocketRoleNoneState.h"
#include "RedpointEOSNetworking/Socket/SocketSubsystemRedpointEOS.h"
#include "RedpointEOSNetworking/Stats.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(4230421139, Redpoint::EOS::Networking::Socket)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::API::P2P;
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::Networking;
using namespace ::Redpoint::EOS::Networking::Socket;

FSocketRedpointEOS::FSocketRedpointEOS(
    const TSharedRef<FSocketSubsystemRedpointEOS> &InSocketSubsystem,
    FPlatformHandle InPlatformHandle,
    FSocketRedpointEOSMemoryId InSocketMemoryId,
    const FString &InSocketDescription)
    : FSocket(ESocketType::SOCKTYPE_Datagram, InSocketDescription, ProtocolType)
    , bClosing(false)
    , bClosed(false)
    , bAlreadyClosedByRemoteHost(false)
    , Role(InSocketSubsystem->RoleInstance_None)
    , RoleState(MakeShared<FSocketRoleNoneState>())
    , PlatformHandle(InPlatformHandle)
    , BindAddress(nullptr)
    , SocketMemoryId(InSocketMemoryId)
    , SocketKey()
    , ReceivedPacketsQueue()
    , ReceivedPacketsQueueCount(0)
    , SocketSubsystem(InSocketSubsystem)
    , OnIncomingConnection()
    , OnConnectionAccepted()
    , OnConnectionClosed()
    , bEnableReliableSending(false)
{
    UE_LOG(
        LogRedpointEOSSocketLifecycle,
        Verbose,
        TEXT("%d: Socket created as: RoleInstance_None"),
        this->SocketMemoryId);
}

FSocketRedpointEOS::~FSocketRedpointEOS()
{
    UE_LOG(LogRedpointEOSSocketLifecycle, Verbose, TEXT("%d: Socket destroyed."), this->SocketMemoryId);

    this->RemoveSocketKeyIfSet();
}

void FSocketRedpointEOS::UpdateSocketKey(const FSocketConnectionMapKey &InSocketKey)
{
    this->RemoveSocketKeyIfSet();

    this->SocketKey = MakeShared<FSocketConnectionMapKey>(InSocketKey);
    this->SocketSubsystem->AssignedSockets.Add(InSocketKey, this->AsSharedRedpointEOS());

    UE_LOG(LogRedpointEOSSocket, Verbose, TEXT("%s: Added to AssignedSockets."), *InSocketKey.ToString());
}

void FSocketRedpointEOS::RemoveSocketKeyIfSet()
{
    if (this->SocketKey.IsValid())
    {
        this->SocketSubsystem->AssignedSockets.Remove(*this->SocketKey);
        UE_LOG(LogRedpointEOSSocket, Verbose, TEXT("%s: Removed from AssignedSockets."), *this->SocketKey->ToString());

        // Check if this was the last socket connection to a given user.
        bool bFoundOtherRequiredConnection = false;
        for (const auto &RemainingSocket : this->SocketSubsystem->AssignedSockets)
        {
            if (RemainingSocket.Key.IsIdenticalExcludingChannel(*this->SocketKey))
            {
                bFoundOtherRequiredConnection = true;
                break;
            }
        }
        if (!bFoundOtherRequiredConnection)
        {
            // We must not call CloseConnection if the socket has already been closed by the remote host (i.e. we got
            // FNotifyPeerConnectionClosed), otherwise the EOS SDK will call FNotifyPeerConnectionClosed again in
            // response to the CloseConnection call below and crash on the next tick.
            if (!this->bAlreadyClosedByRemoteHost)
            {
                UE_LOG(
                    LogRedpointEOSSocket,
                    Verbose,
                    TEXT("%s: Closing connection because the last socket to the remote endpoint has been closed."),
                    *this->SocketKey->ToString());

                auto CloseResult = FCloseConnection::Execute(
                    this->PlatformHandle,
                    FCloseConnection::Options{
                        GetProductUserId(this->SocketKey->GetLocalUserId()),
                        GetProductUserId(this->SocketKey->GetRemoteUserId()),
                        this->SocketKey->GetSymmetricSocketId()});
                if (CloseResult != EOS_EResult::EOS_Success)
                {
                    UE_LOG(
                        LogRedpointEOSSocket,
                        Error,
                        TEXT("%s: %s"),
                        *this->SocketKey->ToString(),
                        *ConvertError(ANSI_TO_TCHAR(__FUNCTION__), TEXT("Failed to close connection."), CloseResult)
                             .ToLogString());
                }
            }
            else
            {
                UE_LOG(
                    LogRedpointEOSSocket,
                    Verbose,
                    TEXT("%s: Skipping close connection call because this connection has already been closed by the "
                         "remote host."),
                    *this->SocketKey->ToString());
            }
        }

        this->SocketKey.Reset();
    }
}

bool FSocketRedpointEOS::Shutdown(ESocketShutdownMode Mode)
{
    UE_LOG(LogRedpointEOSNetworking, Error, TEXT("FSocketRedpointEOS::Shutdown is not supported."));
    return false;
}

bool FSocketRedpointEOS::Close()
{
    if (this->bClosing || this->bClosed)
    {
        UE_LOG(LogRedpointEOSSocketLifecycle, Verbose, TEXT("%d: Socket already closed."), this->SocketMemoryId);
        return true;
    }

    this->bClosing = true;

    if (!this->Role->Close(*this))
    {
        this->bClosing = false;
        return false;
    }

    UE_LOG(
        LogRedpointEOSSocketLifecycle,
        Verbose,
        TEXT("%d: Socket unbound on close. Bind address has been reset."),
        this->SocketMemoryId);

    this->RemoveSocketKeyIfSet();

    this->BindAddress.Reset();

    // Ensure UEOSNetConnection is aware that the socket is closed and that any connections
    // still using this socket should also close.
    this->bClosed = true;
    UE_LOG(
        LogRedpointEOSSocketLifecycle,
        Verbose,
        TEXT("%d: Socket is now marked as closed for FNetConnectionImpl."),
        this->SocketMemoryId);

    return true;
}

bool FSocketRedpointEOS::Bind(const FInternetAddr &Addr)
{
    if (Addr.GetProtocolType() != ProtocolType)
    {
        UE_LOG(LogRedpointEOSNetworking, Error, TEXT("FSocketRedpointEOS::Bind called with non-P2P address."));
        return false;
    }

    if (this->BindAddress.IsValid())
    {
        UE_LOG(LogRedpointEOSNetworking, Error, TEXT("FSocketRedpointEOS::Bind called on already bound socket."));
        return false;
    }

    this->BindAddress = MakeShared<FInternetAddrRedpointEOS>();
    this->BindAddress->SetRawIp(Addr.GetRawIp());
    check(IsProductUserId(this->BindAddress->GetUserId()));

    UE_LOG(
        LogRedpointEOSSocketLifecycle,
        Verbose,
        TEXT("%d: Socket bound to: %s"),
        this->SocketMemoryId,
        *this->BindAddress->ToString(true));

    return true;
}

bool FSocketRedpointEOS::Connect(const FInternetAddr &Addr)
{
    return this->Role->Connect(*this, (const FInternetAddrRedpointEOS &)Addr);
}

bool FSocketRedpointEOS::Listen(int32 MaxBacklog)
{
    return this->Role->Listen(*this, MaxBacklog);
}

bool FSocketRedpointEOS::WaitForPendingConnection(bool &bHasPendingConnection, const FTimespan &WaitTime)
{
    UE_LOG(LogRedpointEOSNetworking, Error, TEXT("FSocketRedpointEOS::WaitForPendingConnection not supported."));
    return false;
}

bool FSocketRedpointEOS::HasPendingData(uint32 &PendingDataSize)
{
    REDPOINT_EOS_SCOPE_CYCLE_COUNTER(Socket, HasPendingData);

#if !(defined(UE_BUILD_SHIPPING) && UE_BUILD_SHIPPING)
    if (CVarOnDemandPacketDispatch.GetValueOnGameThread())
    {
        this->SocketSubsystem->Tick_ReceiveP2PPackets(0.0f);
    }
#endif

    return this->Role->HasPendingData(*this, PendingDataSize);
}

FSocket *FSocketRedpointEOS::Accept(const FString &InSocketDescription)
{
    UE_LOG(
        LogRedpointEOSNetworking,
        Error,
        TEXT(
            "FSocketRedpointEOS::Accept is not intended to be called by client code; it should only be used internally "
            "within the socket subsystem. Instead, if you want to be notified when a client connects to a listening "
            "socket, bind to the OnConnectionAccepted event on the listening socket instead."));
    return nullptr;
}

FSocket *FSocketRedpointEOS::Accept(FInternetAddr &InRemoteAddr, const FString &InSocketDescription)
{
    FSocketRedpointEOS *NewSocket = (FSocketRedpointEOS *)this->SocketSubsystem->CreateSocket(
        FName(TEXT("EOSSocket")),
        InSocketDescription,
        ProtocolType);
    check(NewSocket != nullptr);

    if (this->Role->Accept(*this, NewSocket->AsSharedRedpointEOS(), (const FInternetAddrRedpointEOS &)InRemoteAddr))
    {
        this->OnConnectionAccepted.ExecuteIfBound(
            this->AsSharedRedpointEOS(),
            NewSocket->AsSharedRedpointEOS(),
            this->BindAddress->GetUserId(),
            ((FInternetAddrRedpointEOS &)InRemoteAddr).GetUserId());
        return NewSocket;
    }

    UE_LOG(LogRedpointEOSNetworking, Error, TEXT("FSocketRedpointEOS::Accept failed."));
    this->SocketSubsystem->DestroySocket(NewSocket);
    return nullptr;
}

bool FSocketRedpointEOS::SendTo(const uint8 *Data, int32 Count, int32 &BytesSent, const FInternetAddr &Destination)
{
    REDPOINT_EOS_SCOPE_CYCLE_COUNTER(Socket, SendTo);

#if !(defined(UE_BUILD_SHIPPING) && UE_BUILD_SHIPPING)
    if (CVarEnablePacketTiming.GetValueOnGameThread())
    {
        SIZE_T AdjustedCount = static_cast<SIZE_T>(Count) + sizeof(int64);

        uint8 *TimedData = static_cast<uint8 *>(FMemory::Malloc(AdjustedCount));
        int64 *Timing = reinterpret_cast<int64 *>(TimedData);
        *Timing = FDateTime::UtcNow().GetTicks();
        FMemory::Memcpy(TimedData + sizeof(int64), Data, static_cast<SIZE_T>(Count));

        int32 TimedBytesSent;
        bool Result = this->Role->SendTo(
            *this,
            TimedData,
            static_cast<int32>(AdjustedCount),
            TimedBytesSent,
            (const FInternetAddrRedpointEOS &)Destination);

        BytesSent = FMath::Max(0, TimedBytesSent - static_cast<int32>(sizeof(int64)));
        FMemory::Free(TimedData);
        return Result;
    }
#endif

    return this->Role->SendTo(*this, Data, Count, BytesSent, (const FInternetAddrRedpointEOS &)Destination);
}

bool FSocketRedpointEOS::Send(const uint8 *Data, int32 Count, int32 &BytesSent)
{
    UE_LOG(LogRedpointEOSNetworking, Error, TEXT("FSocketRedpointEOS::Send is not supported."));
    return false;
}

bool FSocketRedpointEOS::RecvFrom(
    uint8 *Data,
    int32 BufferSize,
    int32 &BytesRead,
    FInternetAddr &Source,
    ESocketReceiveFlags::Type Flags)
{
    REDPOINT_EOS_SCOPE_CYCLE_COUNTER(Socket, RecvFrom);

    if (Flags != ESocketReceiveFlags::None)
    {
        UE_LOG(
            LogRedpointEOSNetworking,
            Error,
            TEXT("FSocketRedpointEOS::RecvFrom does not support calling with non-None flags."));
        return false;
    }

#if !(defined(UE_BUILD_SHIPPING) && UE_BUILD_SHIPPING)
    if (CVarOnDemandPacketDispatch.GetValueOnGameThread())
    {
        this->SocketSubsystem->Tick_ReceiveP2PPackets(0.0f);
    }
#endif

    return this->Role->RecvFrom(*this, Data, BufferSize, BytesRead, (FInternetAddrRedpointEOS &)Source, Flags);
}

bool FSocketRedpointEOS::Recv(uint8 *Data, int32 BufferSize, int32 &BytesRead, ESocketReceiveFlags::Type Flags)
{
    UE_LOG(LogRedpointEOSNetworking, Error, TEXT("FSocketRedpointEOS::Recv is not supported."));
    return false;
}

bool FSocketRedpointEOS::RecvMulti(FRecvMulti &MultiData, ESocketReceiveFlags::Type Flags)
{
    UE_LOG(LogRedpointEOSNetworking, Error, TEXT("FSocketRedpointEOS::RecvMulti not supported."));
    return false;
}

bool FSocketRedpointEOS::Wait(ESocketWaitConditions::Type Condition, FTimespan WaitTime)
{
    UE_LOG(LogRedpointEOSNetworking, Error, TEXT("FSocketRedpointEOS::Wait not supported."));
    return false;
}

ESocketConnectionState FSocketRedpointEOS::GetConnectionState()
{
    UE_LOG(LogRedpointEOSNetworking, Error, TEXT("FSocketRedpointEOS::GetConnectionState is not supported."));
    return ESocketConnectionState::SCS_NotConnected;
}

void FSocketRedpointEOS::GetAddress(FInternetAddr &OutAddr)
{
    if (this->BindAddress.IsValid())
    {
        OutAddr.SetRawIp(this->BindAddress->GetRawIp());
        return;
    }

    UE_LOG(LogRedpointEOSNetworking, Error, TEXT("FSocketRedpointEOS::GetAddress called on non-bound socket."));
}

bool FSocketRedpointEOS::GetPeerAddress(FInternetAddr &OutAddr)
{
    return this->Role->GetPeerAddress(*this, (FInternetAddrRedpointEOS &)OutAddr);
}

bool FSocketRedpointEOS::SetNonBlocking(bool bIsNonBlocking)
{
    UE_LOG(LogRedpointEOSNetworking, Error, TEXT("FSocketRedpointEOS::SetNonBlocking is not supported."));
    return false;
}

bool FSocketRedpointEOS::SetBroadcast(bool bAllowBroadcast)
{
    UE_LOG(LogRedpointEOSNetworking, Error, TEXT("FSocketRedpointEOS::SetBroadcast is not supported."));
    return false;
}

bool FSocketRedpointEOS::SetNoDelay(bool bIsNoDelay)
{
    UE_LOG(LogRedpointEOSNetworking, Error, TEXT("FSocketRedpointEOS::SetNoDelay is not supported."));
    return false;
}

bool FSocketRedpointEOS::JoinMulticastGroup(const FInternetAddr &GroupAddress)
{
    UE_LOG(LogRedpointEOSNetworking, Error, TEXT("FSocketRedpointEOS::JoinMulticastGroup is not supported."));
    return false;
}

bool FSocketRedpointEOS::JoinMulticastGroup(const FInternetAddr &GroupAddress, const FInternetAddr &InterfaceAddress)
{
    UE_LOG(LogRedpointEOSNetworking, Error, TEXT("FSocketRedpointEOS::JoinMulticastGroup is not supported."));
    return false;
}

bool FSocketRedpointEOS::LeaveMulticastGroup(const FInternetAddr &GroupAddress)
{
    UE_LOG(LogRedpointEOSNetworking, Error, TEXT("FSocketRedpointEOS::LeaveMulticastGroup is not supported."));
    return false;
}

bool FSocketRedpointEOS::LeaveMulticastGroup(const FInternetAddr &GroupAddress, const FInternetAddr &InterfaceAddress)
{
    UE_LOG(LogRedpointEOSNetworking, Error, TEXT("FSocketRedpointEOS::LeaveMulticastGroup is not supported."));
    return false;
}

bool FSocketRedpointEOS::SetMulticastLoopback(bool bLoopback)
{
    UE_LOG(LogRedpointEOSNetworking, Error, TEXT("FSocketRedpointEOS::SetMulticastLoopback is not supported."));
    return false;
}

bool FSocketRedpointEOS::SetMulticastTtl(uint8 TimeToLive)
{
    UE_LOG(LogRedpointEOSNetworking, Error, TEXT("FSocketRedpointEOS::SetMulticastTtl is not supported."));
    return false;
}

bool FSocketRedpointEOS::SetMulticastInterface(const FInternetAddr &InterfaceAddress)
{
    UE_LOG(LogRedpointEOSNetworking, Error, TEXT("FSocketRedpointEOS::SetMulticastInterface is not supported."));
    return false;
}

bool FSocketRedpointEOS::SetReuseAddr(bool bAllowReuse)
{
    UE_LOG(LogRedpointEOSNetworking, Error, TEXT("FSocketRedpointEOS::SetReuseAddr is not supported."));
    return false;
}

bool FSocketRedpointEOS::SetLinger(bool bShouldLinger, int32 Timeout)
{
    UE_LOG(LogRedpointEOSNetworking, Error, TEXT("FSocketRedpointEOS::SetLinger is not supported."));
    return false;
}

bool FSocketRedpointEOS::SetRecvErr(bool bUseErrorQueue)
{
    UE_LOG(LogRedpointEOSNetworking, Error, TEXT("FSocketRedpointEOS::SetRecvErr is not supported."));
    return false;
}

bool FSocketRedpointEOS::SetSendBufferSize(int32 Size, int32 &NewSize)
{
    UE_LOG(LogRedpointEOSNetworking, Error, TEXT("FSocketRedpointEOS::SetSendBufferSize is not supported."));
    return false;
}

bool FSocketRedpointEOS::SetReceiveBufferSize(int32 Size, int32 &NewSize)
{
    UE_LOG(LogRedpointEOSNetworking, Error, TEXT("FSocketRedpointEOS::SetReceiveBufferSize is not supported."));
    return false;
}

int32 FSocketRedpointEOS::GetPortNo()
{
    UE_LOG(LogRedpointEOSNetworking, Error, TEXT("FSocketRedpointEOS::GetPortNo is not supported."));
    return 0;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()
