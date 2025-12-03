// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSNetworking/NetDriverImpl.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Misc/CommandLine.h"
#include "OnlineBeacon.h"
#include "RedpointEOSAntiCheat/Logging.h"
#include "RedpointEOSConfig/Config.h"
#include "RedpointEOSControlMessages.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSCore/Utils/WorldResolution.h"
#include "RedpointEOSIdentity/IdentitySystem.h"
#include "RedpointEOSIpNetConnection.h"
#include "RedpointEOSNetConnection.h"
#include "RedpointEOSNetworking/Logging.h"
#include "RedpointEOSNetworking/MultiResolve/SocketSubsystemMultiIpResolve.h"
#include "RedpointEOSNetworking/NetConnectionImpl.h"
#include "RedpointEOSNetworking/NetworkingSystem.h"
#include "RedpointEOSNetworking/Socket/P2PChannel.h"
#include "RedpointEOSNetworking/Socket/ProtocolType.h"
#include "RedpointEOSNetworking/Socket/SocketSubsystemRedpointEOS.h"
#include "RedpointEOSNetworking/Stats.h"
#include "Sockets.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(911548221, Redpoint::EOS::Networking)
{
using namespace ::Redpoint::EOS::AntiCheat;
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::Core::Utils;
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::Networking;
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
using namespace ::Redpoint::EOS::Networking::Socket;
#endif

FNetDriverImpl::FNetDriverImpl(
    const TSoftObjectPtr<UWorld> &InWorld,
    const TSoftObjectPtr<URedpointEOSNetDriver> &InNetDriver,
    const FPlatformHandle &InPlatformHandle,
    const FPlatformOptionalRefCountedHandle &InPlatformOptionalRefCountedHandle)
    : World(InWorld)
    , NetDriver(InNetDriver)
    , PlatformHandle(InPlatformHandle)
    , PlatformOptionalRefCountedHandle(InPlatformOptionalRefCountedHandle)
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    , Socket(nullptr)
    , IsDeferringToIpNetDriver(false)
    , RegisteredListeningAccount()
#endif
#if !UE_BUILD_SHIPPING
    , MultiIpResolveSubsystem(
          (InPlatformHandle->GetEnvironment()->Config->GetUseFallbackAddressesInDevelopment() &&
           !FParse::Param(FCommandLine::Get(), TEXT("emulateeosshipping")))
              ? MakeShared<FSocketSubsystemMultiIpResolve>()
              : TSharedPtr<FSocketSubsystemMultiIpResolve>()

              )
#endif
    , PendingConnectionClose()

{
}

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

URedpointEOSNetConnection *FNetDriverImpl::CreateConnection()
{
    auto *Connection = NewObject<URedpointEOSNetConnection>();
    checkf(Connection, TEXT("Expected connection to be created."));
    Connection->Impl = MakeShared<FNetConnectionImpl>(this->NetDriver, Connection);
    return Connection;
}

#endif

bool FNetDriverImpl::IsOwnedByBeacon()
{
    if (GEngine != nullptr)
    {
        for (const auto &WorldContext : GEngine->GetWorldContexts())
        {
            UWorld *ItWorld = WorldContext.World();
            if (ItWorld != nullptr)
            {
                for (TActorIterator<AOnlineBeacon> It(ItWorld); It; ++It)
                {
                    if (It->GetNetDriver() == this->NetDriver)
                    {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

bool FNetDriverImpl::CanPerformP2PNetworking(const TSharedPtr<FSocketSubsystemRedpointEOS> &InSocketSubsystem)
{
    auto UserId = InSocketSubsystem->GetBindingProductUserId_P2POnly();
    return UserId.IsSet() && IsProductUserId(*UserId);
}

bool FNetDriverImpl::CreateEOSSocket(
    const TSharedPtr<FSocketSubsystemRedpointEOS> &InSocketSubsystem,
    const FString &InDescription,
    const FURL &InURL,
    bool bListening,
    TSharedPtr<FSocketRedpointEOS> &OutSocket,
    UE::Online::FAccountId &OutBindingUserId)
{
    // Get the socket name from the networking stack.
    FString SocketName = InSocketSubsystem->GetSocketName(bListening, InURL);

    // Retrieve the local binding address.
    auto BindingUserId = InSocketSubsystem->GetBindingProductUserId_P2POnly();
    if (!BindingUserId.IsSet() || !IsProductUserId(*BindingUserId))
    {
        // No valid local user for binding.
        return false;
    }
    TSharedPtr<FInternetAddrRedpointEOS> BindAddr =
        StaticCastSharedRef<FInternetAddrRedpointEOS>(InSocketSubsystem->CreateInternetAddr());
    BindAddr->SetFromParameters(*BindingUserId, SocketName, (uint8)(InURL.Port % (int32)(P2PChannelIdModulo)));

    // Make the actual socket.
    UE_LOG(
        LogRedpointEOSNetworking,
        VeryVerbose,
        TEXT("Creating P2P socket for network driver with bind address '%s'."),
        *BindAddr->ToString(true));
    auto *NewSocket = static_cast<FSocketRedpointEOS *>(
        InSocketSubsystem->CreateSocket(FName(TEXT("EOSSocket")), InDescription, ProtocolType));
    check(NewSocket != nullptr);
    verify(NewSocket->Bind(*BindAddr));

    // Call Listen or Connect based on what this socket is used for.
    if (bListening)
    {
        UE_LOG(LogRedpointEOSNetworking, VeryVerbose, TEXT("Requesting that the new P2P socket start listening."));
        verify(NewSocket->Listen(0));
    }
    else
    {
        auto DestAddr = StaticCastSharedRef<FInternetAddrRedpointEOS>(InSocketSubsystem->CreateInternetAddr());
        bool bIsValid = false;
        DestAddr->SetIp(*InURL.Host, bIsValid);
        DestAddr->SetPort(InURL.Port);
        UE_LOG(
            LogRedpointEOSNetworking,
            VeryVerbose,
            TEXT("Connecting the new P2P socket to remote address '%s'."),
            *DestAddr->ToString(true));
        check(bIsValid);
        verify(NewSocket->Connect(*DestAddr));
        check(DestAddr->GetPort() == BindAddr->GetPort());
    }

    OutSocket = NewSocket->AsSharedRedpointEOS();
    OutBindingUserId = *BindingUserId;
    return true;
}

#endif

void FNetDriverImpl::SendAntiCheatData(
    const TSharedRef<FAntiCheatSession> &Session,
    const UE::Online::FAccountId &SourceUserId,
    const UE::Online::FAccountId &TargetUserId,
    const uint8 *Data,
    uint32_t Size)
{
#define FUNCTION_CONTEXT_MACRO "SendAntiCheatData(Session: %p, SourceUserId: %s, TargetUserId: %s, Data: ..., Size: %u)"
#define FUNCTION_CONTEXT_ARGS                                                                                          \
    &Session.Get(), *GetUserIdDebugString(SourceUserId), *GetUserIdDebugString(TargetUserId), Size

    UE_LOG(LogRedpointEOSAntiCheat, VeryVerbose, TEXT(FUNCTION_CONTEXT_MACRO ": Called"), FUNCTION_CONTEXT_ARGS);

    checkf(
        Session == this->AntiCheatSession.ToSharedRef(),
        TEXT("Network send must match Anti-Cheat session of net driver!"));

    FUniqueNetIdRepl SourceUserIdRepl(*GetUniqueNetIdRef(SourceUserId));
    FUniqueNetIdRepl TargetUserIdRepl(*GetUniqueNetIdRef(TargetUserId));
    TArray<uint8> DataArr(Data, static_cast<int32>(Size));

    if (this->NetDriver->ServerConnection)
    {
        // Connected to remote server.
        FNetControlMessage<NMT_EOS_AntiCheatMessage>::Send(
            this->NetDriver->ServerConnection,
            SourceUserIdRepl,
            TargetUserIdRepl,
            DataArr);
        UE_LOG(
            LogRedpointEOSAntiCheat,
            VeryVerbose,
            TEXT(FUNCTION_CONTEXT_MACRO ": Server connection: %p: Sent EAC packet to server."),
            FUNCTION_CONTEXT_ARGS,
            this->NetDriver->ServerConnection.Get());
    }
    else
    {
        // Send to remote client.
        bool bDidSendToRemoteClient = false;
        for (const auto &ClientConnection : this->NetDriver->ClientConnections)
        {
            if (IsValid(ClientConnection) && !ClientConnection->bPendingDestroy && TargetUserId.IsValid() &&
                ClientConnection->PlayerId.IsValid() &&
                *GetUniqueNetIdRef(TargetUserId) == *ClientConnection->PlayerId.GetUniqueNetId())
            {
                FNetControlMessage<NMT_EOS_AntiCheatMessage>::Send(
                    ClientConnection,
                    SourceUserIdRepl,
                    TargetUserIdRepl,
                    DataArr);
                bDidSendToRemoteClient = true;
                UE_LOG(
                    LogRedpointEOSAntiCheat,
                    VeryVerbose,
                    TEXT(FUNCTION_CONTEXT_MACRO ": Client connection: %p: Sent EAC packet to client."),
                    FUNCTION_CONTEXT_ARGS,
                    ClientConnection.Get());
                break;
            }
            else
            {
                bool bIsValid = IsValid(ClientConnection);
                bool bIsPendingDestroy = bIsValid && ClientConnection->bPendingDestroy;
                bool bTargetUserIdIsValid = TargetUserId.IsValid();
                bool bClientConnectionUserIdIsValid = bIsValid && ClientConnection->PlayerId.IsValid();
                bool bTargetUserIdIsConnectionUserId =
                    bTargetUserIdIsValid && bClientConnectionUserIdIsValid &&
                    *GetUniqueNetIdRef(TargetUserId) == *ClientConnection->PlayerId.GetUniqueNetId();

                UE_LOG(
                    LogRedpointEOSAntiCheat,
                    VeryVerbose,
                    TEXT(FUNCTION_CONTEXT_MACRO
                         ": Client connection: %p: Connection does not meet requirements for send (bIsValid: %s, "
                         "bIsPendingDestroy: %s, bTargetUserIdIsValid: %s, bClientConnectionUserIdIsValid: %s, "
                         "bTargetUserIdIsConnectionUserId: %s)"),
                    FUNCTION_CONTEXT_ARGS,
                    ClientConnection.Get(),
                    bIsValid ? TEXT("true") : TEXT("false"),
                    bIsPendingDestroy ? TEXT("true") : TEXT("false"),
                    bTargetUserIdIsValid ? TEXT("true") : TEXT("false"),
                    bClientConnectionUserIdIsValid ? TEXT("true") : TEXT("false"),
                    bTargetUserIdIsConnectionUserId ? TEXT("true") : TEXT("false"));
            }
        }
        if (!bDidSendToRemoteClient)
        {
            UE_LOG(
                LogRedpointEOSAntiCheat,
                Warning,
                TEXT(FUNCTION_CONTEXT_MACRO
                     ": There is no client connection for the target user, so this EAC packet was dropped."),
                FUNCTION_CONTEXT_ARGS);
        }
    }
#undef FUNCTION_CONTEXT_MACRO
#undef FUNCTION_CONTEXT_ARGS
}

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

bool FNetDriverImpl::Socket_OnIncomingConnection(
    const TSharedRef<FSocketRedpointEOS> &,
    const UE::Online::FAccountId &LocalUser,
    const UE::Online::FAccountId &RemoteUser)
{
    REDPOINT_EOS_SCOPE_CYCLE_COUNTER(NetDriver, OnIncomingConnection);

    // If we are a server, use the notify interface to determine if we want to accept the connection.
    if (this->NetDriver->ServerConnection == nullptr)
    {
        if (auto SocketPinned = this->Socket.Pin())
        {
            if (SocketPinned.IsValid())
            {
                if (this->NetDriver->Notify != nullptr &&
                    this->NetDriver->Notify->NotifyAcceptingConnection() == EAcceptConnection::Accept)
                {
                    UE_LOG(
                        LogRedpointEOSNetworking,
                        Verbose,
                        TEXT("%s allowed a new incoming connection."),
                        ANSI_TO_TCHAR(__FUNCTION__));
                    return true;
                }

                UE_LOG(
                    LogRedpointEOSNetworking,
                    Warning,
                    TEXT("%s rejected a new incoming connection."),
                    ANSI_TO_TCHAR(__FUNCTION__));
                return false;
            }
        }
    }

    // If we are a client, we'll receive the incoming connection call when the server starts talking back to
    // us for the first time. Always accept these connections. Calling NotifyAcceptingConnection would fail since
    // we're the ones who initiated the outbound connection.
    return true;
}

void FNetDriverImpl::Socket_OnConnectionAccepted(
    const TSharedRef<FSocketRedpointEOS> &ListeningSocket,
    const TSharedRef<FSocketRedpointEOS> &AcceptedSocket,
    const UE::Online::FAccountId &LocalUser,
    const UE::Online::FAccountId &RemoteUser)
{
    REDPOINT_EOS_SCOPE_CYCLE_COUNTER(NetDriver, OnConnectionAccepted);

    if (auto SocketPinned = this->Socket.Pin())
    {
        if (SocketPinned.IsValid())
        {
            check(&ListeningSocket.Get() == SocketPinned.Get());

            if (this->NetDriver->ServerConnection == nullptr)
            {
                // Get the address.
                auto SocketSubsystem = this->PlatformHandle->GetSystem<FSocketSubsystemRedpointEOS>();
                auto IncomingAddr =
                    StaticCastSharedRef<FInternetAddrRedpointEOS>(SocketSubsystem->CreateInternetAddr());
                verify(AcceptedSocket->GetPeerAddress(*IncomingAddr));

#if !UE_BUILD_SHIPPING
                if (this->MultiIpResolveSubsystem.IsValid())
                {
                    for (auto ExistingClientConnection : this->NetDriver->ClientConnections)
                    {
                        auto ExistingClientConnectionEOS = Cast<URedpointEOSNetConnection>(ExistingClientConnection);
                        if (ExistingClientConnectionEOS)
                        {
                            if (auto ExistingSocketPinned = ExistingClientConnectionEOS->Impl->Socket.Pin())
                            {
                                auto ExistingAddr = SocketSubsystem->CreateInternetAddr();
                                if (ExistingSocketPinned->GetPeerAddress(*ExistingAddr))
                                {
                                    // Check to make sure the peer address of the newly accepted socket does not
                                    // conflict with any existing client connection. If it does, this is a bug
                                    // because the new connection should only be accepted after the cleanup has
                                    // finished running in ::Socket_OnConnectionClosed if this is a reconnect.
                                    check(!(*ExistingAddr == *IncomingAddr));
                                }
                            }
                        }
                    }
                }
#endif

                // We are a server and we have a new client connection, construct
                // the connection object and add it to our list of client connections.
                //
                // Server connections to clients *MUST* be created in the USOCK_Open status,
                // otherwise handshaking (challenge response) won't be handled correctly
                // on the server.
                auto *ClientConnection = this->CreateConnection();
                check(ClientConnection);
                ClientConnection->InitRemoteConnection(
                    this->NetDriver.Get(),
                    &AcceptedSocket.Get(),
                    this->NetDriver->World ? this->NetDriver->World->URL : FURL(),
                    *IncomingAddr,
                    USOCK_Open);
                this->NetDriver->Notify->NotifyAcceptedConnection(ClientConnection);
                this->NetDriver->AddClientConnection(ClientConnection);

                UE_LOG(
                    LogRedpointEOSNetworking,
                    Verbose,
                    TEXT("%s accepted a new client connection."),
                    ANSI_TO_TCHAR(__FUNCTION__));
            }
            else
            {
                // We are a client and we just got a connection back from the server. Update our state
                // to USOCK_Open.
                this->NetDriver->ServerConnection->SetConnectionState(USOCK_Open);
            }
        }
    }
}

void FNetDriverImpl::Socket_OnConnectionClosed(
    const TSharedRef<FSocketRedpointEOS> &ListeningSocket,
    const TSharedRef<FSocketRedpointEOS> &ClosedSocket,
    const FString &CloseSource)
{
    REDPOINT_EOS_SCOPE_CYCLE_COUNTER(NetDriver, OnConnectionClosed);

    UE_LOG(
        LogRedpointEOSNetworking,
        Verbose,
        TEXT("%s received notification that a socket is closing from '%s'."),
        ANSI_TO_TCHAR(__FUNCTION__),
        *CloseSource);

    if (!this->NetDriver.IsValid())
    {
        return;
    }

    // When a remote socket is closed on a listening socket, we need to remove the client connection from the driver. If
    // we don't do this, then the server won't handshake correctly on the next connection.
    if (this->NetDriver->ServerConnection == nullptr)
    {
        for (auto i = this->NetDriver->ClientConnections.Num() - 1; i >= 0; i--)
        {
            auto *ClientConnection = Cast<URedpointEOSNetConnection>(this->NetDriver->ClientConnections[i]);
            if (ClientConnection)
            {
                // Is it the same socket as the one that is closing?
                if (ClientConnection->Impl->Socket.Pin().Get() == &ClosedSocket.Get())
                {
                    UE_LOG(
                        LogRedpointEOSNetworking,
                        Verbose,
                        TEXT("%s removed a client connection because the underlying socket is closing."),
                        ANSI_TO_TCHAR(__FUNCTION__));
                    ClientConnection->CleanUp();
                }
            }
        }
    }
}

#endif

TOptional<UE::Online::FAccountId> FNetDriverImpl::GetFirstSignedInUser() const
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
    return FirstAccount->GetUserId();
}

void FNetDriverImpl::RequestConnectionClose(URedpointEOSNetConnection *InConnection)
{
    if (!IsValid(InConnection))
    {
        UE_LOG(
            LogRedpointEOSNetworking,
            Error,
            TEXT("%s was not passed a valid connection value, so it will not be added to the pending close list."),
            ANSI_TO_TCHAR(__FUNCTION__));
        return;
    }

    if (!this->PendingConnectionClose.Contains(InConnection))
    {
        UE_LOG(
            LogRedpointEOSNetworking,
            Verbose,
            TEXT("%s: Connection %p was added to the pending connection close list."),
            ANSI_TO_TCHAR(__FUNCTION__),
            InConnection);
        this->PendingConnectionClose.Add(InConnection);
    }
}

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

bool FNetDriverImpl::IsIpMode() const
{
    return this->IsDeferringToIpNetDriver;
}

TOptional<UE::Online::FAccountId> FNetDriverImpl::GetRegisteredListeningUser() const
{
    return this->RegisteredListeningAccount;
}

TSharedPtr<FSocketRedpointEOS> FNetDriverImpl::GetRedpointEOSSocket() const
{
    return this->Socket.Pin();
}

#endif

ENetDriverRole FNetDriverImpl::GetEOSRole()
{
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    checkf(
        this->NetDriver.IsValid(),
        TEXT("Expected NetDriver to be valid when calling %s!"),
        ANSI_TO_TCHAR(__FUNCTION__));
    if (!this->NetDriver.IsValid())
    {
        return ENetDriverRole::ListenServer;
    }

    if (this->IsDeferringToIpNetDriver)
    {
        if (IsValid(this->NetDriver->ServerConnection))
        {
            return ENetDriverRole::ClientConnectedToDedicatedServer;
        }
        else
        {
            return ENetDriverRole::DedicatedServer;
        }
    }
    else
    {
        if (IsValid(this->NetDriver->ServerConnection))
        {
            return ENetDriverRole::ClientConnectedToListenServer;
        }
        else
        {
            return ENetDriverRole::ListenServer;
        }
    }
#else
    return ENetDriverRole::DedicatedServer;
#endif
}

TOptional<FPlatformHandle> FNetDriverImpl::GetPlatformHandle()
{
    return this->PlatformHandle;
}

bool FNetDriverImpl::IsAvailable() const
{
    // @ todo
    return true;
}

bool FNetDriverImpl::IsNetResourceValid()
{
    if (!this->NetDriver.IsValid())
    {
        return false;
    }

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    if (this->IsDeferringToIpNetDriver)
#endif
    {
        return this->NetDriver->UIpNetDriver_IsNetResourceValid();
    }

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    return this->Socket.IsValid();
#endif
}

// @note: At the point GetSocketSubsystem is called, the TSoftObjectPtr to the NetDriver inside the Impl may no longer
// be valid because the NetDriver is calling GetSocketSubsystem as part of LowLevelDestroy (the IpNetDriver does this).
// Thus we pass the raw pointer as a parameter instead.
ISocketSubsystem *FNetDriverImpl::GetSocketSubsystem(URedpointEOSNetDriver *NetDriverPtr)
{
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    if (this->IsDeferringToIpNetDriver)
#endif
    {
#if !UE_BUILD_SHIPPING
        if (this->PlatformHandle->GetEnvironment()->Config->GetUseFallbackAddressesInDevelopment() &&
            !FParse::Param(FCommandLine::Get(), TEXT("emulateeosshipping")))
        {
            return this->MultiIpResolveSubsystem.Get();
        }
        else
        {
            return NetDriverPtr->UIpNetDriver_GetSocketSubsystem();
        }
#else
        return NetDriverPtr->UIpNetDriver_GetSocketSubsystem();
#endif
    }

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    return &this->PlatformHandle->GetSystem<FSocketSubsystemRedpointEOS>().Get();
#endif
}

bool FNetDriverImpl::InitConnect(FNetworkNotify *InNotify, const FURL &InConnectURL, FString &Error)
{
    REDPOINT_EOS_SCOPE_CYCLE_COUNTER(NetDriver, InitConnect);

    checkf(
        this->NetDriver.IsValid(),
        TEXT("Expected NetDriver to be valid when calling %s!"),
        ANSI_TO_TCHAR(__FUNCTION__));
    if (!this->NetDriver.IsValid())
    {
        return false;
    }

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    checkf(
        !this->IsDeferringToIpNetDriver,
        TEXT("IsDeferringToIpNetDriver should not be set at the point of InitConnect being called!"));
#endif

    // Ensure that the control channel isn't messed up.
    if (!this->NetDriver->IsKnownChannelName(NAME_Control))
    {
        UE_LOG(
            LogRedpointEOSNetworking,
            Error,
            TEXT("The networking driver does not have a control channel, which will prevent connections from working. "
                 "This indicates that either your configuration is incorrect, or the engine is bugged."));
        return false;
    }

    // Get systems from the platform handle.
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    auto SocketSubsystem = this->PlatformHandle->GetSystem<FSocketSubsystemRedpointEOS>();
#endif
    auto IdentitySystem = this->PlatformHandle->GetSystem<IIdentitySystem>();
    auto AntiCheatSystem = this->PlatformHandle->GetSystem<IAntiCheatSystem>();

    // We need to be able to mutate the connection URL if we are fixing it up for the editor.
    FURL ConnectURL = InConnectURL;

    // Check that the config is set up correctly.
    check(this->NetDriver->RelevantTimeout >= 0.0f);

    // Cache whether we are being used by a beacon.
    this->bIsOwnedByBeacon = this->IsOwnedByBeacon();

#if WITH_EDITOR
    // When launching a multiplayer game in the editor through play-in-editor, the editor always tells clients
    // to connect to 127.0.0.1 on a given port. However, if the server is listening on a P2P address, this won't
    // succeed.
    //
    // Try to detect this scenario and redirect the connection URL to the correct EOS P2P URL for the game.
    if (ConnectURL.Host == TEXT("127.0.0.1") && GEngine != nullptr)
    {
        FWorldContext *WorldContextFromPending =
            GEngine->GetWorldContextFromPendingNetGameNetDriver(this->NetDriver.Get());
        if (WorldContextFromPending != nullptr)
        {
            if (WorldContextFromPending->WorldType == EWorldType::PIE)
            {
                // Try to locate the host world context.
                for (auto CandidateWorldContext : GEngine->GetWorldContexts())
                {
                    if (CandidateWorldContext.WorldType == EWorldType::PIE && CandidateWorldContext.PIEInstance == 0)
                    {
                        // If this check fails, it means the logic inside FOnlineIdentityInterfaceEOS to make clients
                        // wait for the server to be ready is not correct.
                        if (CandidateWorldContext.bWaitingOnOnlineSubsystem)
                        {
                            UE_LOG(
                                LogRedpointEOSNetworking,
                                Error,
                                TEXT("PIE client attempted to connect to localhost, but host PIE context is still "
                                     "waiting for online subsystem. This is a bug!"));
                            break;
                        }
                        if (CandidateWorldContext.ActiveNetDrivers.Num() == 0 ||
                            !IsValid(CandidateWorldContext.ActiveNetDrivers[0].NetDriver))
                        {
                            UE_LOG(
                                LogRedpointEOSNetworking,
                                Warning,
                                TEXT("PIE client is attempting to connect to localhost, but the first PIE instance "
                                     "doesn't appear to be a listening server. If the host you are connecting to is "
                                     "listening over EOS P2P, this connection will fail."));
                            break;
                        }

                        auto HostingNetDriver =
                            Cast<URedpointEOSNetDriver>(CandidateWorldContext.ActiveNetDrivers[0].NetDriver);

                        if (IsValid(HostingNetDriver))
                        {
                            if (HostingNetDriver->Impl->IsIpMode())
                            {
                                // The network driver is listening on an IP address; the default 127.0.0.1 behaviour is
                                // fine.
                            }
                            else if (auto NetSocketPinned = HostingNetDriver->Impl->GetRedpointEOSSocket())
                            {
                                // The network driver is listening on a P2P address, we need to modify the target
                                // address.
                                auto BindAddr = this->PlatformHandle->GetSystem<FSocketSubsystemRedpointEOS>()
                                                    ->CreateInternetAddr();
                                NetSocketPinned->GetAddress(*BindAddr);
                                ConnectURL.Host = BindAddr->ToString(false);
                                // @todo: Also set the ConnectURL port here? It should already match.
                            }
                        }

                        break;
                    }
                }
            }
        }
    }
#endif

    // If the host of the connection URL does not end with .eosp2p, then this is an IP-based connection.
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    if (!ConnectURL.Host.EndsWith(TEXT(".eosp2p")))
#endif
    {
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
        this->IsDeferringToIpNetDriver = true;
#endif
#if !UE_BUILD_SHIPPING
        if (this->MultiIpResolveSubsystem.IsValid())
        {
            if (ConnectURL.Host.Contains(TEXT(",")))
            {
                // Modify connection settings so we can get through the list of
                // multiple IP addresses quickly.
                this->NetDriver->InitialConnectTimeout =
                    this->PlatformHandle->GetEnvironment()->Config->GetFallbackAddressTimeoutSecondsInDevelopment();
                this->NetDriver->bNoTimeouts = false;
            }
        }
#endif
        this->NetDriver->NetConnectionClass = URedpointEOSIpNetConnection::StaticClass();

        if (!this->bIsOwnedByBeacon)
        {
            // Try to find the connecting user of this net driver.
            auto FirstUser = IdentitySystem->GetUser(0);
            if (FirstUser.IsValid())
            {
                // Create the Anti-Cheat session for IP.
                checkf(!this->AntiCheatSession.IsValid(), TEXT("Expect Anti-Cheat session to not already be created."));
                this->AntiCheatSession = AntiCheatSystem->CreateSession(
                    false,
                    FirstUser->GetUserId(),
                    true,
                    TOptional<UE::Online::FAccountId>(),
                    FString::Printf(TEXT("%s:%d"), *ConnectURL.Host, ConnectURL.Port));
                if (!this->AntiCheatSession.IsValid())
                {
                    UE_LOG(LogRedpointEOSNetworking, Error, TEXT("Net driver failed to set up Anti-Cheat session."));
                    return false;
                }
                checkf(
                    !AntiCheatSystem->OnSendNetworkMessage().IsBound(),
                    TEXT("IAntiCheat::OnSendNetworkMessage should not be bound yet."));
                AntiCheatSystem->OnSendNetworkMessage().BindSP(this, &FNetDriverImpl::SendAntiCheatData);
            }
        }

        bool bConnectSuccess = this->NetDriver->UIpNetDriver_InitConnect(InNotify, ConnectURL, Error);
        if (!bConnectSuccess)
        {
            return false;
        }

        UE_LOG(
            LogRedpointEOSNetworking,
            Verbose,
            TEXT("%s completed successfully for IP connection."),
            ANSI_TO_TCHAR(__FUNCTION__));
        return true;
    }

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    // If the connection timeouts are set too low for any reason, fix it now. EOS P2P is not fast to connect, and if you
    // set the timeout too low, connections will always fail.
    if (this->NetDriver->InitialConnectTimeout < 120.0f)
    {
        UE_LOG(
            LogRedpointEOSNetworking,
            Warning,
            TEXT("The connection timeout of the EOS networking driver is configured to be %.2f, but this is too low to "
                 "establish connections over EOS P2P. We are fixing it to a minimum of 120 seconds to ensure that "
                 "clients can connect successfully."),
            static_cast<double>(this->NetDriver->InitialConnectTimeout));
        this->NetDriver->InitialConnectTimeout = 120.0f;
    }

    // Check if the user can do P2P networking as a client.
    if (!this->CanPerformP2PNetworking(SocketSubsystem))
    {
        UE_LOG(
            LogRedpointEOSNetworking,
            Warning,
            TEXT("FNetDriverImpl: Attempting to connect to a P2P address, but CanPerformP2PNetworking reports "
                 "that P2P networking is not available."));
    }

    // Perform common initialization.
    if (!this->NetDriver->UNetDriver_InitBase(true, InNotify, ConnectURL, false, Error))
    {
        UE_LOG(
            LogRedpointEOSNetworking,
            Error,
            TEXT("FNetDriverImpl: InitConnect failed while setting up base information"));
        return false;
    }

    // Create the socket.
    TSharedPtr<FSocketRedpointEOS> CreatedSocket;
    UE::Online::FAccountId BindingUserId;
    if (!this->CreateEOSSocket(
            SocketSubsystem,
            TEXT("Unreal client (EOS)"),
            ConnectURL,
            false,
            CreatedSocket,
            BindingUserId))
    {
        return false;
    }

    // Set up Anti-Cheat for P2P.
    if (!this->bIsOwnedByBeacon)
    {
        // Create the Anti-Cheat session.
        checkf(!this->AntiCheatSession.IsValid(), TEXT("Expect Anti-Cheat session to not already be created."));
        auto DestAddr = StaticCastSharedRef<FInternetAddrRedpointEOS>(SocketSubsystem->CreateInternetAddr());
        verifyf(CreatedSocket->GetPeerAddress(*DestAddr), TEXT("Peer address can be read for connecting P2P socket"));
        this->AntiCheatSession = AntiCheatSystem->CreateSession(
            false,
            BindingUserId,
            false,
            DestAddr->GetUserId(),
            FString::Printf(TEXT("%s:%d"), *ConnectURL.Host, ConnectURL.Port));
        if (!this->AntiCheatSession.IsValid())
        {
            UE_LOG(LogRedpointEOSNetworking, Error, TEXT("Net driver failed to set up Anti-Cheat session."));
            return false;
        }
        checkf(
            !AntiCheatSystem->OnSendNetworkMessage().IsBound(),
            TEXT("IAntiCheat::OnSendNetworkMessage should not be bound yet."));
        AntiCheatSystem->OnSendNetworkMessage().BindSP(this, &FNetDriverImpl::SendAntiCheatData);
    }

    // Set LocalAddr, which contains the address we bound on.
    this->NetDriver->LocalAddr = StaticCastSharedRef<FInternetAddrRedpointEOS>(SocketSubsystem->CreateInternetAddr());
    CreatedSocket->GetAddress(*this->NetDriver->LocalAddr);

    // Listen for connection open / close.
    CreatedSocket->OnConnectionClosed.BindSP(this, &FNetDriverImpl::Socket_OnConnectionClosed);
    CreatedSocket->OnConnectionAccepted.BindSP(this, &FNetDriverImpl::Socket_OnConnectionAccepted);
    CreatedSocket->OnIncomingConnection.BindSP(this, &FNetDriverImpl::Socket_OnIncomingConnection);

    // Create the network connection to the server.
    this->NetDriver->ServerConnection = this->CreateConnection();
    this->NetDriver->ServerConnection
        ->InitLocalConnection(this->NetDriver.Get(), CreatedSocket.Get(), ConnectURL, USOCK_Pending);
    this->NetDriver->CreateInitialClientChannels();

    this->Socket = CreatedSocket;

    UE_LOG(
        LogRedpointEOSNetworking,
        Verbose,
        TEXT("%s completed successfully for P2P connection."),
        ANSI_TO_TCHAR(__FUNCTION__));
    return true;
#endif
}

bool FNetDriverImpl::InitListen(FNetworkNotify *InNotify, FURL &ListenURL, bool bReuseAddressAndPort, FString &Error)
{
    REDPOINT_EOS_SCOPE_CYCLE_COUNTER(NetDriver, InitListen);

    checkf(
        this->NetDriver.IsValid(),
        TEXT("Expected NetDriver to be valid when calling %s!"),
        ANSI_TO_TCHAR(__FUNCTION__));
    if (!this->NetDriver.IsValid())
    {
        return false;
    }

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    checkf(
        !this->IsDeferringToIpNetDriver,
        TEXT("IsDeferringToIpNetDriver should not be set at the point of InitListen being called!"));
#endif

    // Ensure that the control channel isn't messed up.
    if (!this->NetDriver->IsKnownChannelName(NAME_Control))
    {
        UE_LOG(
            LogRedpointEOSNetworking,
            Error,
            TEXT("The networking driver does not have a control channel, which will prevent connections from working. "
                 "This indicates that either your configuration is incorrect, or the engine is bugged."));
        return false;
    }

    // Get systems from the platform handle.
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    auto SocketSubsystem = this->PlatformHandle->GetSystem<FSocketSubsystemRedpointEOS>();
#endif
    auto IdentitySystem = this->PlatformHandle->GetSystem<IIdentitySystem>();
    auto AntiCheatSystem = this->PlatformHandle->GetSystem<IAntiCheatSystem>();
    auto NetworkingSystem = this->PlatformHandle->GetSystem<INetworkingSystem>();

    // Check that the config is set up correctly.
    check(this->NetDriver->RelevantTimeout >= 0.0f);

    // Cache whether we are being used by a beacon.
    this->bIsOwnedByBeacon = this->IsOwnedByBeacon();

    // Determine what mode to run networking in.
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    FString ListenMode = FString(ListenURL.GetOption(TEXT("NetMode="), TEXT("auto"))).ToLower();
    if (!GIsAutomationTesting && ListenMode != TEXT("auto"))
    {
        UE_LOG(
            LogRedpointEOSNetworking,
            Warning,
            TEXT("The 'NetMode=' listen option is no longer supported and is now always ignored unless running "
                 "automation tests. The Redpoint EOS plugin will automatically pick the most suitable networking "
                 "layer."));
    }
    bool P2PAvailable = this->CanPerformP2PNetworking(SocketSubsystem);
    bool bUseIPDriver = !P2PAvailable;
    if (GIsAutomationTesting)
    {
        bUseIPDriver = ListenMode == TEXT("forceip") || (!P2PAvailable && ListenMode != TEXT("forcep2p"));
    }
    if (bUseIPDriver)
#endif
    {
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
        this->IsDeferringToIpNetDriver = true;
#endif
        this->NetDriver->NetConnectionClass = URedpointEOSIpNetConnection::StaticClass();

        if (!this->NetDriver->UIpNetDriver_InitListen(InNotify, ListenURL, bReuseAddressAndPort, Error))
        {
            return false;
        }

        auto IpBindingUserId = this->GetFirstSignedInUser();
        if (IpBindingUserId.IsSet() && this->NetDriver->GetSocket() != nullptr)
        {
            // Only register the IP-based address if the binding user is valid. If the binding user isn't
            // valid, then the user can't use the session API to create sessions anyway, so we don't need
            // to worry about it not being registered.
            TSharedRef<FInternetAddr> IpAddr = this->NetDriver->UIpNetDriver_GetSocketSubsystem()->CreateInternetAddr();
            this->NetDriver->GetSocket()->GetAddress(*IpAddr);
            IpAddr->SetPort(this->NetDriver->GetSocket()->GetPortNo());
            TArray<TSharedPtr<FInternetAddr>> DevIpAddrPtrs;
            TArray<TSharedRef<const FInternetAddr>> DevIpAddrRefs;
#if !UE_BUILD_SHIPPING
            if (this->MultiIpResolveSubsystem.IsValid())
            {
                if (this->GetSocketSubsystem(this->NetDriver.Get())->GetLocalAdapterAddresses(DevIpAddrPtrs))
                {
                    TSharedRef<FInternetAddr> LocalhostAddr =
                        this->GetSocketSubsystem(this->NetDriver.Get())->CreateInternetAddr();
                    LocalhostAddr->SetLoopbackAddress();
                    DevIpAddrPtrs.Add(LocalhostAddr);
                }
            }
            for (const auto &Ptr : DevIpAddrPtrs)
            {
                if (Ptr.IsValid())
                {
                    DevIpAddrRefs.Add(Ptr.ToSharedRef());
                }
            }
#endif
            if (!this->bIsOwnedByBeacon)
            {
                NetworkingSystem->AddListeningState(IpBindingUserId.GetValue(), IpAddr, DevIpAddrRefs);
            }
            this->RegisteredListeningAccount = IpBindingUserId;
        }

        if (!this->bIsOwnedByBeacon)
        {
            // Create the Anti-Cheat session for IP.
            checkf(!this->AntiCheatSession.IsValid(), TEXT("Expect Anti-Cheat session to not already be created."));
            this->AntiCheatSession = AntiCheatSystem->CreateSession(
                true,
                // IP-based connections protected by Anti-Cheat *must* assume dedicated server, because connecting
                // clients will not be able to know if the target server is logged in or not when they are
                // connecting.
                GetDedicatedServerAccountId(),
                true,
                TOptional<UE::Online::FAccountId>(),
                TEXT(""));
            if (!this->AntiCheatSession.IsValid())
            {
                UE_LOG(LogRedpointEOSNetworking, Error, TEXT("Net driver failed to set up Anti-Cheat session."));
                return false;
            }
            checkf(
                !AntiCheatSystem->OnSendNetworkMessage().IsBound(),
                TEXT("IAntiCheat::OnSendNetworkMessage should not be bound yet."));
            AntiCheatSystem->OnSendNetworkMessage().BindSP(this, &FNetDriverImpl::SendAntiCheatData);
        }

        // IP-based connection has finished being set up.
        UE_LOG(
            LogRedpointEOSNetworking,
            Verbose,
            TEXT("%s completed successfully for IP connection."),
            ANSI_TO_TCHAR(__FUNCTION__));
        return true;
    }
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    else if (GIsAutomationTesting && ListenMode == TEXT("forcep2p") && !P2PAvailable)
    {
        UE_LOG(
            LogRedpointEOSNetworking,
            Error,
            TEXT("NetMode=ForceP2P but peer-to-peer networking is not available (because the user is not signed into "
                 "an EOS account)"));
        return false;
    }

    // Perform common initialization.
    if (!this->NetDriver->UNetDriver_InitBase(false, InNotify, ListenURL, bReuseAddressAndPort, Error))
    {
        UE_LOG(
            LogRedpointEOSNetworking,
            Error,
            TEXT("FNetDriverImpl: InitListen failed while setting up base information"));
        return false;
    }

    // We used to call InitConnectionlessHandler here, but this code path is only
    // for P2P connections, and we no longer use the packet handler infrastructure
    // for those types of connections.

    // Create the socket.
    TSharedPtr<FSocketRedpointEOS> CreatedSocket;
    UE::Online::FAccountId BindingUserId;
    if (!this->CreateEOSSocket(
            SocketSubsystem,
            TEXT("Unreal server (EOS)"),
            ListenURL,
            true,
            CreatedSocket,
            BindingUserId))
    {
        return false;
    }

    // Set up Anti-Cheat for P2P.
    if (!this->bIsOwnedByBeacon)
    {
        // Create the Anti-Cheat session.
        checkf(!this->AntiCheatSession.IsValid(), TEXT("Expect Anti-Cheat session to not already be created."));
        this->AntiCheatSession =
            AntiCheatSystem->CreateSession(true, BindingUserId, false, TOptional<UE::Online::FAccountId>(), TEXT(""));
        if (!this->AntiCheatSession.IsValid())
        {
            UE_LOG(LogRedpointEOSNetworking, Error, TEXT("Net driver failed to set up Anti-Cheat session."));
            return false;
        }
        checkf(
            !AntiCheatSystem->OnSendNetworkMessage().IsBound(),
            TEXT("IAntiCheat::OnSendNetworkMessage should not be bound yet."));
        AntiCheatSystem->OnSendNetworkMessage().BindSP(this, &FNetDriverImpl::SendAntiCheatData);
    }

    // Set LocalAddr, which contains the address we bound on.
    check(!this->NetDriver->LocalAddr.IsValid());
    this->NetDriver->LocalAddr = StaticCastSharedRef<FInternetAddrRedpointEOS>(SocketSubsystem->CreateInternetAddr());
    CreatedSocket->GetAddress(*this->NetDriver->LocalAddr);

    // Listen for connection open / close.
    CreatedSocket->OnConnectionClosed.BindSP(this, &FNetDriverImpl::Socket_OnConnectionClosed);
    CreatedSocket->OnConnectionAccepted.BindSP(this, &FNetDriverImpl::Socket_OnConnectionAccepted);
    CreatedSocket->OnIncomingConnection.BindSP(this, &FNetDriverImpl::Socket_OnIncomingConnection);

    // Register this socket as a listening socket on the online subsystem.
    if (!this->bIsOwnedByBeacon)
    {
        NetworkingSystem->AddListeningState(
            BindingUserId,
            this->NetDriver->LocalAddr.ToSharedRef(),
            TArray<TSharedRef<const FInternetAddr>>());
    }
    this->RegisteredListeningAccount = BindingUserId;

    this->Socket = CreatedSocket;

    UE_LOG(
        LogRedpointEOSNetworking,
        Verbose,
        TEXT("%s completed successfully for P2P connection."),
        ANSI_TO_TCHAR(__FUNCTION__));
    return true;
#endif
}

void FNetDriverImpl::InitConnectionlessHandler()
{
    checkf(
        this->NetDriver.IsValid(),
        TEXT("Expected NetDriver to be valid when calling %s!"),
        ANSI_TO_TCHAR(__FUNCTION__));
    if (!this->NetDriver.IsValid())
    {
        return;
    }

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    if (this->IsDeferringToIpNetDriver)
#endif
    {
        // IP passthrough needs normal connectionless handler, including
        // stateless connection handler.
        this->NetDriver->UIpNetDriver_InitConnectionlessHandler();
    }
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    else
    {
        // We do not use the packet handler infrastructure on P2P connections;
        // we don't need stateless connection handling or encryption handling.
    }
#endif
}

void FNetDriverImpl::TickDispatch(float DeltaTime)
{
    REDPOINT_EOS_SCOPE_CYCLE_COUNTER(NetDriver, TickDispatch);

    checkf(
        this->NetDriver.IsValid(),
        TEXT("Expected NetDriver to be valid when calling %s!"),
        ANSI_TO_TCHAR(__FUNCTION__));
    if (!this->NetDriver.IsValid())
    {
        return;
    }

    // If we are connecting over an IP address, defer to the IpNetDriver.
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    if (this->IsDeferringToIpNetDriver)
#endif
    {
        this->NetDriver->UIpNetDriver_TickDispatch(DeltaTime);
        return;
    }

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    // Before we do anything else, close connections that are pending close.
    if (this->PendingConnectionClose.Num() > 0)
    {
        UE_LOG(
            LogRedpointEOSNetworking,
            Verbose,
            TEXT("%s: There are %d pending connections to close."),
            ANSI_TO_TCHAR(__FUNCTION__),
            this->PendingConnectionClose.Num());
    }
    for (auto ConnectionToClose : this->PendingConnectionClose)
    {
        if (ConnectionToClose.IsValid())
        {
            UE_LOG(
                LogRedpointEOSNetworking,
                Verbose,
                TEXT("%s: Calling Close on the UNetConnection that is pending close."),
                ANSI_TO_TCHAR(__FUNCTION__));
            ConnectionToClose->Close();
        }
        else
        {
            UE_LOG(
                LogRedpointEOSNetworking,
                Error,
                TEXT("%s: Pending connection for close was not valid when TickDispatch was called."),
                ANSI_TO_TCHAR(__FUNCTION__));
        }
    }
    this->PendingConnectionClose.Empty();

    // Tick the UNetDriver (skipping over the UIpNetDriver when in P2P mode).
    {
        REDPOINT_EOS_SCOPE_CYCLE_COUNTER(NetDriver, BaseTickDispatch);

        this->NetDriver->UNetDriver_TickDispatch(DeltaTime);
    }

    // Get systems from the platform handle.
    auto SocketSubsystem = this->PlatformHandle->GetSystem<FSocketSubsystemRedpointEOS>();

    while (true)
    {
        REDPOINT_EOS_DWORD_COUNTER_STAT_INCREMENT(P2P, ReceivedLoopIters);
        REDPOINT_EOS_INT_COUNTER_INCREMENT(P2P, ReceivedLoopIters);

        // We MUST scope the socket here before ReceivePacketFromDriver gets called. In ReceivePacketFromDriver, we can
        // end up processing user code which might want to disconnect the connection or destroy a beacon. If we don't
        // scope the socket here, then the check(EOSSocketPtr.IsUnique()); inside FSocketSubsystemEOSFull::DestroySocket
        // will fail.
        uint8 *Buffer = nullptr;
        int32 BytesRead = 0;
        auto ReceivedAddr = StaticCastSharedRef<FInternetAddrRedpointEOS>(SocketSubsystem->CreateInternetAddr());
        if (TSharedPtr<FSocketRedpointEOS> SocketPinned = this->Socket.Pin())
        {
            if (!SocketPinned.IsValid())
            {
                // Socket is no longer valid.
                break;
            }

            uint32 PendingDataSize = 0;
            if (!SocketPinned->HasPendingData(PendingDataSize) || PendingDataSize == 0)
            {
                // No more pending data.
                break;
            }

            Buffer = (uint8 *)FMemory::Malloc(PendingDataSize);
            if (!SocketPinned->RecvFrom(
                    Buffer,
                    static_cast<int32>(PendingDataSize),
                    BytesRead,
                    *ReceivedAddr,
                    ESocketReceiveFlags::None))
            {
                FMemory::Free(Buffer);
                UE_LOG(LogRedpointEOSNetworking, Error, TEXT("FNetDriverImpl::TickDispatch: Socket RecvFrom failed!"));
                break;
            }
        }
        else
        {
            // Socket is no longer valid.
            break;
        }

        if (BytesRead == 0)
        {
            FMemory::Free(Buffer);
            UE_LOG(
                LogRedpointEOSNetworking,
                Error,
                TEXT("FNetDriverImpl::TickDispatch: Socket RecvFrom read 0 bytes of data!"));
            break;
        }

        REDPOINT_EOS_DWORD_COUNTER_STAT_INCREMENT(P2P, ReceivedPackets);
        REDPOINT_EOS_DWORD_COUNTER_STAT_INCREMENT_BY(P2P, ReceivedBytes, BytesRead);
        REDPOINT_EOS_INT_COUNTER_INCREMENT(P2P, ReceivedPackets);
        REDPOINT_EOS_INT_COUNTER_ADD(P2P, ReceivedPackets, BytesRead);

        if (this->NetDriver->ServerConnection != nullptr)
        {
            // This is a client connection to a remote server.
            Cast<URedpointEOSNetConnection>(this->NetDriver->ServerConnection)
                ->Impl->ReceivePacketFromDriver(ReceivedAddr, Buffer, BytesRead);
        }
        else
        {
            // This is data coming in from a remote client to a local listen/dedicated server.
            bool bFoundClientConnection = false;
            for (UNetConnection *ClientConnection : this->NetDriver->ClientConnections)
            {
                auto *EOSClientConnection = Cast<URedpointEOSNetConnection>(ClientConnection);

                // Fetch the client address; again, the pinned socket must be pinned before ReceivePacketFromDriver is
                // called.
                auto ClientAddr = StaticCastSharedRef<FInternetAddrRedpointEOS>(SocketSubsystem->CreateInternetAddr());
                if (TSharedPtr<FSocketRedpointEOS> ClientSocketPinned = EOSClientConnection->Impl->Socket.Pin())
                {
                    if (ClientSocketPinned.IsValid())
                    {
                        ClientSocketPinned->GetPeerAddress(*ClientAddr);
                    }
                }

                // If this is the right client, have it receive the data.
                if (ClientAddr->CompareEndpoints(*ReceivedAddr))
                {
                    bFoundClientConnection = true;
                    EOSClientConnection->Impl->ReceivePacketFromDriver(ReceivedAddr, Buffer, BytesRead);
                    break;
                }
            }

            if (!bFoundClientConnection)
            {
                UE_LOG(
                    LogRedpointEOSNetworking,
                    Warning,
                    TEXT("FNetDriverImpl::TickDispatch: Could not find client connection to receive data!"));
            }
        }

        FMemory::Free(Buffer);
    }
#endif
}

void FNetDriverImpl::TickFlush(float DeltaTime)
{
    checkf(
        this->NetDriver.IsValid(),
        TEXT("Expected NetDriver to be valid when calling %s!"),
        ANSI_TO_TCHAR(__FUNCTION__));
    if (!this->NetDriver.IsValid())
    {
        return;
    }

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    if (!this->IsDeferringToIpNetDriver)
    {
        // Before we flush, close connections that are pending close.
        if (this->PendingConnectionClose.Num() > 0)
        {
            UE_LOG(
                LogRedpointEOSNetworking,
                Verbose,
                TEXT("%s: There are %d pending connections to close."),
                ANSI_TO_TCHAR(__FUNCTION__),
                this->PendingConnectionClose.Num());
        }
        for (auto ConnectionToClose : this->PendingConnectionClose)
        {
            if (ConnectionToClose.IsValid())
            {
                UE_LOG(
                    LogRedpointEOSNetworking,
                    Verbose,
                    TEXT("%s: Calling Close on the UNetConnection that is pending close."),
                    ANSI_TO_TCHAR(__FUNCTION__));
                ConnectionToClose->Close();
            }
            else
            {
                UE_LOG(
                    LogRedpointEOSNetworking,
                    Error,
                    TEXT("%s: Pending connection for close was not valid when TickDispatch was called."),
                    ANSI_TO_TCHAR(__FUNCTION__));
            }
        }
        this->PendingConnectionClose.Empty();
    }
#endif

    this->NetDriver->UIpNetDriver_TickFlush(DeltaTime);
}

void FNetDriverImpl::LowLevelSend(
    TSharedPtr<const FInternetAddr> Address,
    void *Data,
    int32 CountBits,
    FOutPacketTraits &Traits)
{
    checkf(
        this->NetDriver.IsValid(),
        TEXT("Expected NetDriver to be valid when calling %s!"),
        ANSI_TO_TCHAR(__FUNCTION__));
    if (!this->NetDriver.IsValid())
    {
        return;
    }

    // If we are connecting over an IP address, defer to the IpNetDriver.
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    if (this->IsDeferringToIpNetDriver)
#endif
    {
        this->NetDriver->UIpNetDriver_LowLevelSend(Address, Data, CountBits, Traits);
        return;
    }

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    TSharedPtr<FSocketRedpointEOS> SocketPinned = this->Socket.Pin();
    if (SocketPinned.IsValid() && Address.IsValid() && Address->IsValid())
    {
        const uint8 *SendData = reinterpret_cast<const uint8 *>(Data);

        // We don't use the packet handler infrastructure, so there's no
        // need to mutate the packet data with the handler.

        if (CountBits > 0)
        {
            int32 BytesToSend = FMath::DivideAndRoundUp(CountBits, 8);
            int32 SentBytes = 0;

            // Our sendto will find the correct socket to send over.
#if REDPOINT_EOS_ENABLE_SOCKET_LEVEL_NETWORK_TRACING
            FNetConnectionImpl::LogPacket(this->NetDriver.Get(), Address, false, SendData, BytesToSend);
#endif
            if (!SocketPinned->SendTo(SendData, BytesToSend, SentBytes, *Address))
            {
                UE_LOG(
                    LogRedpointEOSNetworking,
                    Warning,
                    TEXT("FNetDriverImpl::LowLevelSend: Could not send %d data over socket to %s!"),
                    BytesToSend,
                    *Address->ToString(true));
            }
            else
            {
                REDPOINT_EOS_DWORD_COUNTER_STAT_INCREMENT(P2P, SentPackets);
                REDPOINT_EOS_DWORD_COUNTER_STAT_INCREMENT_BY(P2P, SentBytes, BytesToSend);
                REDPOINT_EOS_INT_COUNTER_INCREMENT(P2P, SentPackets);
                REDPOINT_EOS_INT_COUNTER_ADD(P2P, SentPackets, BytesToSend);
            }
        }
    }
    else
    {
        UE_LOG(
            LogRedpointEOSNetworking,
            Error,
            TEXT("FNetDriverImpl::LowLevelSend: Could not send data because either the socket or address is "
                 "invalid!"));
    }
#endif
}

// @note: At the point LowLevelDestroy is called, the TSoftObjectPtr to the NetDriver inside the Impl is no longer
// valid because the NetDriver is marked for garbage collection. Thus we pass the raw pointer as a parameter for use
// inside LowLevelDestroy instead.
void FNetDriverImpl::LowLevelDestroy(URedpointEOSNetDriver *NetDriverPtr)
{
    // Get systems from the platform handle.
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    auto SocketSubsystem = this->PlatformHandle->GetSystem<FSocketSubsystemRedpointEOS>();
#endif
    auto AntiCheatSystem = this->PlatformHandle->GetSystem<IAntiCheatSystem>();
    auto NetworkingSystem = this->PlatformHandle->GetSystem<INetworkingSystem>();

    if (this->AntiCheatSession.IsValid())
    {
        AntiCheatSystem->OnSendNetworkMessage().Unbind();
        if (!AntiCheatSystem->DestroySession(*this->AntiCheatSession))
        {
            UE_LOG(LogRedpointEOSNetworking, Warning, TEXT("Net driver failed to destroy Anti-Cheat session!"));
        }
        this->AntiCheatSession.Reset();
    }

    if (this->RegisteredListeningAccount.IsSet())
    {
        if (NetDriverPtr->LocalAddr.IsValid())
        {
            if (!this->bIsOwnedByBeacon)
            {
                NetworkingSystem->RemoveListeningState(
                    this->RegisteredListeningAccount.GetValue(),
                    NetDriverPtr->LocalAddr.ToSharedRef());
            }
        }
        this->RegisteredListeningAccount.Reset();
    }

    // If we are connecting over an IP address, defer to the IpNetDriver.
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    if (this->IsDeferringToIpNetDriver)
#endif
    {
        NetDriverPtr->UIpNetDriver_LowLevelDestroy();
        return;
    }

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    if (TSharedPtr<FSocketRedpointEOS> SocketPtr = this->Socket.Pin())
    {
        // We must get the pointer and then set SocketPtr to nullptr so that the
        // the socket subsystem has the singular remaining strong reference to the
        // socket.
        auto *SocketRaw = SocketPtr.Get();
        SocketPtr = nullptr;

        // Destroy the socket.
        SocketSubsystem->DestroySocket(SocketRaw);
    }

    this->Socket = nullptr;
#endif
}

}

REDPOINT_EOS_CODE_GUARD_END()