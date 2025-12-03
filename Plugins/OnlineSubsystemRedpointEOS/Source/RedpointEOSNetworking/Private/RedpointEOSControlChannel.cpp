// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSControlChannel.h"

#include "Engine/ActorChannel.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "Misc/Base64.h"
#include "Net/NetPing.h"
#include "RedpointEOSAntiCheat/AntiCheatActionReason.h"
#include "RedpointEOSAntiCheat/AntiCheatSystem.h"
#include "RedpointEOSAntiCheat/Logging.h"
#include "RedpointEOSConfig/Config.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSNetConnection.h"
#include "RedpointEOSNetworking/Auth/Beacon/AuthBeaconPhaseContext.h"
#include "RedpointEOSNetworking/Auth/Connection/AuthConnectionPhaseContext.h"
#include "RedpointEOSNetworking/Auth/Connection/AutomaticEncryptionCommon.h"
#include "RedpointEOSNetworking/Auth/Connection/AutomaticEncryptionPhase.h"
#include "RedpointEOSNetworking/Auth/Login/AntiCheatIntegrityPhase.h"
#include "RedpointEOSNetworking/Auth/Login/AntiCheatProofPhase.h"
#include "RedpointEOSNetworking/Auth/Login/AuthLoginPhase.h"
#include "RedpointEOSNetworking/Auth/Login/AuthLoginPhaseContext.h"
#include "RedpointEOSNetworking/Auth/NetworkHelpers.h"
#include "RedpointEOSNetworking/Auth/Queues/QueuedBeaconEntry.h"
#include "RedpointEOSNetworking/Auth/Queues/QueuedLoginEntry.h"
#include "RedpointEOSNetworking/Auth/Verification/AuthVerificationPhaseContext.h"
#include "RedpointEOSNetworking/Auth/Verification/IdTokenAuthPhase.h"
#include "RedpointEOSNetworking/Auth/Verification/P2PAddressCheckPhase.h"
#include "RedpointEOSNetworking/Auth/Verification/SanctionCheckPhase.h"
#include "RedpointEOSNetworking/NetConnectionImpl.h"
#include "RedpointEOSNetworking/NetDriverRole.h"
#include "RedpointEOSNetworking/Socket/InternetAddrRedpointEOS.h"
#include "RedpointEOSNetworking/Socket/SocketSubsystemRedpointEOS.h"
#if REDPOINT_EOS_UE_5_6_OR_LATER
#include "Interfaces/ReplicatedObjectInterface.h"
#endif
#if REDPOINT_EOS_UE_5_5_OR_LATER
#include "ProfilingDebugging/AssetMetadataTrace.h"
#include "UObject/Package.h"
#if UE_WITH_IRIS
#include "Iris/ReplicationSystem/ReplicationSystem.h"
#endif
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

URedpointEOSControlChannel::URedpointEOSControlChannel(const FObjectInitializer &ObjectInitializer)
    : UControlChannel(ObjectInitializer)
{
    using namespace ::Redpoint::EOS::Networking::Auth::Login;
    using namespace ::Redpoint::EOS::Networking::Auth::Beacon;
    using namespace ::Redpoint::EOS::Networking::Auth::Connection;
    using namespace ::Redpoint::EOS::Networking::Auth::Verification;

    this->AuthConnectionContext.Reset();
    this->QueuedLogins.Reset();
    this->QueuedBeacons.Reset();

    IAutomaticEncryptionCommon::RegisterRoutes(this);
    FAntiCheatIntegrityPhase::RegisterRoutes(this);
    FAntiCheatProofPhase::RegisterRoutes(this);
    FIdTokenAuthPhase::RegisterRoutes(this);
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    FP2PAddressCheckPhase::RegisterRoutes(this);
#endif
    FSanctionCheckPhase::RegisterRoutes(this);
}

void URedpointEOSControlChannel::AddRoute(
    uint8 MessageType,
    const ::Redpoint::EOS::Networking::Auth::FAuthPhaseRoute &Route)
{
    this->Routes.Add(MessageType, Route);
}

TSharedPtr<::Redpoint::EOS::Networking::Auth::Connection::FAuthConnectionPhaseContext> URedpointEOSControlChannel::
    GetAuthConnectionPhaseContext()
{
    using namespace ::Redpoint::EOS::Networking;
    using namespace ::Redpoint::EOS::Networking::Auth;
    using namespace ::Redpoint::EOS::Networking::Auth::Connection;

    if (!this->AuthConnectionContext.IsValid())
    {
        ENetDriverRole Role = FNetworkHelpers::GetRole(Connection);
        if (Role == ENetDriverRole::ClientConnectedToDedicatedServer ||
            Role == ENetDriverRole::ClientConnectedToListenServer)
        {
            // Implicitly create the authentication context on clients, since
            // we don't set them up as we send the NMT_Hello outbound.
            this->AuthConnectionContext = MakeShared<FAuthConnectionPhaseContext>(this);
            TArray<TSharedRef<IAuthConnectionPhase>> Phases;
            Phases.Add(MakeShared<FAutomaticEncryptionPhase>());
            this->AuthConnectionContext->RegisterPhasesForClientRouting(
                Phases,
                FOnClientMustDisconnectFromServer::CreateUObject(
                    this,
                    &URedpointEOSControlChannel::OnClientMustDisconnectFromServer));
            return this->AuthConnectionContext;
        }
    }

    return this->AuthConnectionContext;
}

TSharedPtr<::Redpoint::EOS::Networking::Auth::Verification::FAuthVerificationPhaseContext> URedpointEOSControlChannel::
    GetAuthVerificationPhaseContext(const FUniqueNetIdRepl &InRepl)
{
    using namespace ::Redpoint::EOS::Core::Id;
    using namespace ::Redpoint::EOS::Networking;
    using namespace ::Redpoint::EOS::Networking::Auth;
    using namespace ::Redpoint::EOS::Networking::Auth::Verification;

    if (!InRepl.IsValid() || !InRepl->IsValid() || !IsValid(Connection))
    {
        return nullptr;
    }

    if (this->AuthVerificationContexts.Contains(GetAccountId(InRepl)))
    {
        return this->AuthVerificationContexts[GetAccountId(InRepl)];
    }

    ENetDriverRole Role = FNetworkHelpers::GetRole(Connection);
    if (Role == ENetDriverRole::ClientConnectedToDedicatedServer ||
        Role == ENetDriverRole::ClientConnectedToListenServer)
    {
        // Implicitly create the verification context on clients, since
        // we don't set them up as we send the NMT_Login/NMT_BeaconJoin outbound.
        // We always register all phases, since the server drives what network
        // messages get sent to the clients.
        TSharedRef<FAuthVerificationPhaseContext> Context =
            MakeShared<FAuthVerificationPhaseContext>(this, GetAccountId(InRepl));
        this->AuthVerificationContexts.Add(GetAccountId(InRepl), Context);
        TArray<TSharedRef<IAuthVerificationPhase>> Phases;
        Phases.Add(MakeShared<FIdTokenAuthPhase>());
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
        Phases.Add(MakeShared<FP2PAddressCheckPhase>());
#endif
        Phases.Add(MakeShared<FSanctionCheckPhase>());
        Context->RegisterPhasesForClientRouting(
            Phases,
            FOnClientMustDisconnectFromServer::CreateUObject(
                this,
                &URedpointEOSControlChannel::OnClientMustDisconnectFromServer));
        return this->AuthVerificationContexts[GetAccountId(InRepl)];
    }

    return nullptr;
}

TSharedPtr<::Redpoint::EOS::Networking::Auth::Login::FAuthLoginPhaseContext> URedpointEOSControlChannel::
    GetAuthLoginPhaseContext(const FUniqueNetIdRepl &InRepl)
{
    using namespace ::Redpoint::EOS::Core::Id;
    using namespace ::Redpoint::EOS::Networking;
    using namespace ::Redpoint::EOS::Networking::Auth;
    using namespace ::Redpoint::EOS::Networking::Auth::Login;
    using namespace ::Redpoint::EOS::Networking::Auth::Queues;

    if (!InRepl.IsValid() || !InRepl->IsValid() || !IsValid(Connection))
    {
        return nullptr;
    }

    if (this->QueuedLogins.Contains(GetAccountId(InRepl)))
    {
        return this->QueuedLogins[GetAccountId(InRepl)]->Context;
    }

    ENetDriverRole Role = FNetworkHelpers::GetRole(Connection);
    if (Role == ENetDriverRole::ClientConnectedToDedicatedServer ||
        Role == ENetDriverRole::ClientConnectedToListenServer)
    {
        // Implicitly create the queued login on clients, since
        // we don't set them up as we send the NMT_Login outbound.
        // We always register all phases, since the server drives what network
        // messages get sent to the clients.
        TSharedRef<FAuthLoginPhaseContext> Context = MakeShared<FAuthLoginPhaseContext>(this, GetAccountId(InRepl));
        TArray<TSharedRef<IAuthLoginPhase>> Phases;
        Phases.Add(MakeShared<FAntiCheatProofPhase>());
        Phases.Add(MakeShared<FAntiCheatIntegrityPhase>());
        Context->RegisterPhasesForClientRouting(
            Phases,
            FOnClientMustDisconnectFromServer::CreateUObject(
                this,
                &URedpointEOSControlChannel::OnClientMustDisconnectFromServer));
        auto Entry = MakeShared<FQueuedLoginEntry>(TEXT(""), TEXT(""), GetAccountId(InRepl), TEXT(""), this);
        Entry->SetContext(Context);
        Entry->Track();
        return Context;
    }

    return nullptr;
}

TSharedPtr<::Redpoint::EOS::Networking::Auth::Beacon::FAuthBeaconPhaseContext> URedpointEOSControlChannel::
    GetAuthBeaconPhaseContext(const FUniqueNetIdRepl &InRepl, const FString &InBeaconName)
{
    using namespace ::Redpoint::EOS::Core::Id;
    using namespace ::Redpoint::EOS::Networking;
    using namespace ::Redpoint::EOS::Networking::Auth;
    using namespace ::Redpoint::EOS::Networking::Auth::Beacon;
    using namespace ::Redpoint::EOS::Networking::Auth::Queues;

    if (!InRepl.IsValid() || !InRepl->IsValid() || !IsValid(Connection))
    {
        return nullptr;
    }

    if (this->QueuedBeacons.Contains(GetAccountId(InRepl)))
    {
        auto &Beacons = this->QueuedBeacons[GetAccountId(InRepl)];
        if (Beacons.Contains(InBeaconName))
        {
            return Beacons[InBeaconName]->Context;
        }
    }

    ENetDriverRole Role = FNetworkHelpers::GetRole(Connection);
    if (Role == ENetDriverRole::ClientConnectedToDedicatedServer ||
        Role == ENetDriverRole::ClientConnectedToListenServer)
    {
        // Implicitly create the queued beacon on clients, since
        // we don't set them up as we send the NMT_BeaconJoin outbound.
        // We always register all phases, since the server drives what network
        // messages get sent to the clients.
        TSharedRef<FAuthBeaconPhaseContext> Context =
            MakeShared<FAuthBeaconPhaseContext>(this, GetAccountId(InRepl), InBeaconName);
        TArray<TSharedRef<IAuthBeaconPhase>> Phases;
        Context->RegisterPhasesForClientRouting(
            Phases,
            FOnClientMustDisconnectFromServer::CreateUObject(
                this,
                &URedpointEOSControlChannel::OnClientMustDisconnectFromServer));
        auto Entry = MakeShared<FQueuedBeaconEntry>(InBeaconName, GetAccountId(InRepl), this);
        Entry->SetContext(Context);
        Entry->Track();
        return Context;
    }

    return nullptr;
}

void URedpointEOSControlChannel::On_NMT_Hello(const FOriginalParameters_NMT_Hello &Parameters)
{
    using namespace ::Redpoint::EOS::Config;
    using namespace ::Redpoint::EOS::Core::Id;
    using namespace ::Redpoint::EOS::Networking;
    using namespace ::Redpoint::EOS::Networking::Auth;
    using namespace ::Redpoint::EOS::Networking::Auth::Connection;
    using namespace ::Redpoint::EOS::Networking::Auth::Queues;

    if (this->AuthConnectionContext.IsValid())
    {
        // We're already processing NMT_Hello.
        return;
    }

    TSharedRef<FAuthConnectionPhaseContext> Context = MakeShared<FAuthConnectionPhaseContext>(this);
    this->AuthConnectionContext = Context;
    const IConfig *Config;
    ENetDriverRole Role;
    if (!Context->GetConfig(Config) || !Context->GetRole(Role))
    {
        // This connection has gone away.
        return;
    }

    TArray<TSharedRef<IAuthConnectionPhase>> Phases;

    if (Role == ENetDriverRole::DedicatedServer)
    {
        // GetEnableTrustedDedicatedServers already evaluates the network authentication
        // mode and forces itself to on under Recommended mode.
        if (Config->GetNetworkAuthenticationMode() != ENetworkAuthenticationMode::Off &&
            Config->GetEnableAutomaticEncryptionOnTrustedDedicatedServers())
        {
            bool bAddAutomaticEncryption = true;

#if WITH_EDITOR
            // If we have automatic encryption turned on and we're in the editor, make sure we can read the
            // public/private keypair. If we can't, force automatic encryption off.
            bool bLoadedKey = false;
            hydro_sign_keypair server_key_pair = {};
            FString PublicKey = Config->GetDedicatedServerPublicKey();
            FString PrivateKey = Config->GetDedicatedServerPrivateKey();
            if (!PublicKey.IsEmpty() && !PrivateKey.IsEmpty())
            {
                TArray<uint8> PublicKeyBytes, PrivateKeyBytes;
                if (FBase64::Decode(PublicKey, PublicKeyBytes) && FBase64::Decode(PrivateKey, PrivateKeyBytes))
                {
                    if (PublicKeyBytes.Num() == sizeof(server_key_pair.pk) &&
                        PrivateKeyBytes.Num() == sizeof(server_key_pair.sk))
                    {
                        FMemory::Memcpy(server_key_pair.pk, PublicKeyBytes.GetData(), sizeof(server_key_pair.pk));
                        FMemory::Memcpy(server_key_pair.sk, PrivateKeyBytes.GetData(), sizeof(server_key_pair.sk));
                        bLoadedKey = true;
                    }
                }
            }
            if (!bLoadedKey)
            {
                UE_LOG(
                    LogRedpointEOSNetworking,
                    Warning,
                    TEXT("Automatic encryption and trusted dedicated servers are being turned off, because the "
                         "public/private keypair could not be read "
                         "and you're running an editor build. Ensure the keys are set correctly in your "
                         "configuration, "
                         "otherwise network connections will not work when the game is packaged."));
                bAddAutomaticEncryption = false;
            }
#endif // #if WITH_EDITOR

            if (bAddAutomaticEncryption)
            {
                Phases.Add(MakeShared<FAutomaticEncryptionPhase>());
            }
        }
    }

    Context->OnCompleted().AddUObject(this, &URedpointEOSControlChannel::Finalize_NMT_Hello, Parameters);
    Context->Start(Context, Phases);
}

void URedpointEOSControlChannel::Finalize_NMT_Hello(
    ::Redpoint::EOS::Networking::Auth::EAuthPhaseFailureCode Result,
    const FString &ErrorMessage,
    FOriginalParameters_NMT_Hello Parameters)
{
    using namespace ::Redpoint::EOS::Networking::Auth;

    FString EmptyEncryptionToken = TEXT("");
    FOutBunch Bunch;
    Bunch << Parameters.IsLittleEndian;
    Bunch << Parameters.RemoteNetworkVersion;
    Bunch << Parameters.EncryptionToken;
#if REDPOINT_EOS_UE_5_1_OR_LATER
    Bunch << Parameters.NetworkFeatures;
#endif

    FInBunch InBunch(this->Connection, Bunch.GetData(), Bunch.GetNumBits());
    this->Connection->Driver->Notify->NotifyControlMessage(this->Connection, NMT_Hello, InBunch);

    // Keep the auth connection context if it succeeded.
    if (Result != EAuthPhaseFailureCode::Success)
    {
        this->AuthConnectionContext.Reset();
    }
}

void URedpointEOSControlChannel::On_NMT_EOS_AntiCheatMessage(
    const UE::Online::FAccountId &SourceUserId,
    const UE::Online::FAccountId &TargetUserId,
    const TArray<uint8> &AntiCheatData)
{
    using namespace ::Redpoint::EOS::AntiCheat;
    using namespace ::Redpoint::EOS::API;
    using namespace ::Redpoint::EOS::Core::Id;
    using namespace ::Redpoint::EOS::Networking;
    using namespace ::Redpoint::EOS::Networking::Auth;
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    using namespace ::Redpoint::EOS::Networking::Socket;
#endif

    TSharedPtr<FPlatformInstance> PlatformHandle;
    TSharedPtr<FAntiCheatSession> AntiCheatSession;
    bool bIsOwnedByBeacon;
    FNetworkHelpers::GetAntiCheat(this->Connection, PlatformHandle, AntiCheatSession, bIsOwnedByBeacon);
    if (bIsOwnedByBeacon)
    {
        // Beacons don't do Anti-Cheat.
        UE_LOG(LogRedpointEOSNetworking, Error, TEXT("Ignoring Anti-Cheat network message for beacon."));
        return;
    }
    if (!PlatformHandle.IsValid() || !AntiCheatSession.IsValid())
    {
        // No anti-cheat session.
        UE_LOG(
            LogRedpointEOSNetworking,
            Error,
            TEXT("Received Anti-Cheat network message, but no active Anti-Cheat session."));
        return;
    }

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    ENetDriverRole Role = FNetworkHelpers::GetRole(this->Connection);
    if (Role == ENetDriverRole::DedicatedServer || Role == ENetDriverRole::ListenServer)
    {
#endif
        if (!IsProductUserId(SourceUserId) || !this->VerificationDatabase.Contains(SourceUserId) ||
            (this->VerificationDatabase[SourceUserId] != EUserVerificationStatus::WaitingForAntiCheatIntegrity &&
             this->VerificationDatabase[SourceUserId] != EUserVerificationStatus::Verified))
        {
            // Drop this packet, it's not coming from a verified user.
            UE_LOG(
                LogRedpointEOSNetworking,
                Error,
                TEXT("Dropping Anti-Cheat packet because it's not coming from a verified user or a user that is "
                     "establishing Anti-Cheat integrity. The packet came from %s."),
                *GetUserIdDebugString(SourceUserId));
            return;
        }
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    }
    else if (!this->CachedEACSourceUserId.IsSet())
    {
        if (Role == ENetDriverRole::ClientConnectedToDedicatedServer)
        {
            // Reset to dedicated server ID.
            this->CachedEACSourceUserId = GetDedicatedServerAccountId();
        }
        else if (Role == ENetDriverRole::ClientConnectedToListenServer)
        {
            // Set it to the unique net ID of the server.
            auto LocalNetDriver = Cast<URedpointEOSNetDriver>(this->Connection->Driver);
            if (!IsValid(LocalNetDriver))
            {
                // What?
                return;
            }

            auto SocketEOS = LocalNetDriver->GetImpl()->GetRedpointEOSSocket();
            if (!SocketEOS.IsValid())
            {
                return;
            }

            auto SocketSubsystem =
                LocalNetDriver->GetImpl()->GetPlatformHandle().GetValue()->GetSystem<FSocketSubsystemRedpointEOS>();

            TSharedPtr<FInternetAddrRedpointEOS> PeerAddr =
                StaticCastSharedRef<FInternetAddrRedpointEOS>(SocketSubsystem->CreateInternetAddr());
            verifyf(SocketEOS->GetPeerAddress(*PeerAddr), TEXT("Peer address could not be read for P2P socket"));

            this->CachedEACSourceUserId = PeerAddr->GetUserId();
        }
    }

    auto SourceUserIdResolved = this->CachedEACSourceUserId.Get(SourceUserId);
#else
    auto SourceUserIdResolved = SourceUserId;
#endif

    if (!PlatformHandle->GetSystem<IAntiCheatSystem>()->ReceiveNetworkMessage(
            *AntiCheatSession,
            SourceUserIdResolved,
            TargetUserId,
            AntiCheatData.GetData(),
            static_cast<uint32_t>(AntiCheatData.Num())))
    {
        UE_LOG(
            LogRedpointEOSNetworking,
            Error,
            TEXT("Anti-Cheat network message was not received at control channel level."));
    }
}

void URedpointEOSControlChannel::OnAntiCheatPlayerAuthStatusChanged(
    UE::Online::FAccountId TargetUserId,
    EOS_EAntiCheatCommonClientAuthStatus NewAuthStatus)
{
    using namespace ::Redpoint::EOS::AntiCheat;
    using namespace ::Redpoint::EOS::Core::Id;
    using namespace ::Redpoint::EOS::Networking;
    using namespace ::Redpoint::EOS::Networking::Auth;
    using namespace ::Redpoint::EOS::Networking::Auth::Login;
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    using namespace ::Redpoint::EOS::Networking::Socket;
#endif

    ENetDriverRole Role = FNetworkHelpers::GetRole(Connection);
    if (Role != ENetDriverRole::ClientConnectedToDedicatedServer &&
        Role != ENetDriverRole::ClientConnectedToListenServer)
    {
        if (this->QueuedLogins.Contains(TargetUserId))
        {
            // We make a copy of the array here, since the OnAntiCheatPlayerAuthStatusChanged call might modify the
            // authentication state.
            TArray<TSharedRef<IAuthLoginPhase>> Phases = this->QueuedLogins[TargetUserId]->Context->Phases;
            TSharedRef<FAuthLoginPhaseContext> Context = this->QueuedLogins[TargetUserId]->Context.ToSharedRef();
            for (const auto &Phase : Phases)
            {
                Phase->OnAntiCheatPlayerAuthStatusChanged(Context, NewAuthStatus);
            }
        }
        else
        {
            UE_LOG(
                LogRedpointEOSAntiCheat,
                VeryVerbose,
                TEXT("Dropping auth status change notification on server for '%s' because it doesn't correlate with a "
                     "current network authentication process."),
                *GetUserIdDebugString(TargetUserId));
        }
    }
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    else if (Role == ENetDriverRole::ClientConnectedToListenServer)
    {
        // This is a client requesting a mutual proof from a listen server and waiting
        // for the listen server to be verified based on EAC events.
        auto *EOSConnection = Cast<URedpointEOSNetConnection>(this->Connection);
        if (IsValid(EOSConnection))
        {
            if (auto Pinned = EOSConnection->GetImpl()->Socket.Pin())
            {
                if (Pinned.IsValid())
                {
                    auto Address = MakeShared<FInternetAddrRedpointEOS>();
                    Pinned->GetAddress(*Address);
                    auto LocalUserId = Address->GetUserId();
                    auto LocalUniqueNetIdRepl = GetUniqueNetIdRepl(LocalUserId);
                    auto Context = this->GetAuthLoginPhaseContext(LocalUniqueNetIdRepl);
                    for (const auto &Phase : Context->Phases)
                    {
                        Phase->OnAntiCheatPlayerAuthStatusChanged(Context.ToSharedRef(), NewAuthStatus);
                    }
                }
                else
                {
                    UE_LOG(
                        LogRedpointEOSAntiCheat,
                        Warning,
                        TEXT("Dropping auth status change notification on client for '%s' because we are unable to "
                             "determine our local user ID from the network connection."),
                        *GetUserIdDebugString(TargetUserId));
                }
            }
            else
            {
                UE_LOG(
                    LogRedpointEOSAntiCheat,
                    Warning,
                    TEXT("Dropping auth status change notification on client for '%s' because we are unable to "
                         "determine our local user ID from the network connection."),
                    *GetUserIdDebugString(TargetUserId));
            }
        }
        else
        {
            UE_LOG(
                LogRedpointEOSAntiCheat,
                Warning,
                TEXT("Dropping auth status change notification on client for '%s' because the connection to the server "
                     "is not valid."),
                *GetUserIdDebugString(TargetUserId));
        }
    }
#endif
    else
    {
        UE_LOG(
            LogRedpointEOSAntiCheat,
            Warning,
            TEXT("Dropping auth status change notification on client for '%s' because we are connected to a dedicated "
                 "server."),
            *GetUserIdDebugString(TargetUserId));
    }
}

void URedpointEOSControlChannel::OnAntiCheatPlayerActionRequired(
    UE::Online::FAccountId TargetUserId,
    EOS_EAntiCheatCommonClientAction ClientAction,
    EOS_EAntiCheatCommonClientActionReason ActionReasonCode,
    const FString &ActionReasonDetailsString)
{
    using namespace ::Redpoint::EOS::AntiCheat;
    using namespace ::Redpoint::EOS::Networking;
    using namespace ::Redpoint::EOS::Networking::Auth;
    using namespace ::Redpoint::EOS::Networking::Auth::Login;

    if (this->QueuedLogins.Contains(TargetUserId))
    {
        // We make a copy of the array here, since the OnAntiCheatPlayerActionRequired call might modify the
        // authentication state.
        TArray<TSharedRef<IAuthLoginPhase>> Phases = this->QueuedLogins[TargetUserId]->Context->Phases;
        TSharedRef<FAuthLoginPhaseContext> Context = this->QueuedLogins[TargetUserId]->Context.ToSharedRef();
        for (const auto &Phase : Phases)
        {
            Phase->OnAntiCheatPlayerActionRequired(Context, ClientAction, ActionReasonCode, ActionReasonDetailsString);
        }
    }
    else if (
        IsValid(this->Connection) && TargetUserId == this->Connection->PlayerId &&
        ClientAction == EOS_EAntiCheatCommonClientAction::EOS_ACCCA_RemovePlayer)
    {
        // Send the client a security warning message and close the connection.
        FString DetailsString = FString::Printf(
            TEXT("Disconnected from server due to Anti-Cheat error. Reason: '%s'. Details: '%s'."),
            *EOS_EAntiCheatCommonClientActionReason_ToString(ActionReasonCode),
            *ActionReasonDetailsString);
        UE_LOG(
            LogRedpointEOSNetworking,
            Warning,
            TEXT("A network connection will close due to an Anti-Cheat error. Reason: '%s'. Details: '%s'."),
            *EOS_EAntiCheatCommonClientActionReason_ToString(ActionReasonCode),
            *ActionReasonDetailsString);
        if (FNetworkHelpers::GetRole(this->Connection) == ENetDriverRole::DedicatedServer ||
            FNetworkHelpers::GetRole(this->Connection) == ENetDriverRole::ListenServer)
        {
            // Clients understand NMT_SecurityViolation.
            FNetControlMessage<NMT_SecurityViolation>::Send(Connection, DetailsString);
        }
        else
        {
            FNetControlMessage<NMT_Failure>::Send(Connection, DetailsString);
        }
        Connection->FlushNet(true);
        Connection->Close();
    }
}

/**
 * Starts the authentication sequence. This *only* gets called on the server, so it doesn't set up the authentication
 * phases on clients. Refer to the implicit context creation in the Get*PhaseContext functions above.
 */
void URedpointEOSControlChannel::StartAuthentication(
    const UE::Online::FAccountId &IncomingUser,
    const TSharedRef<::Redpoint::EOS::Networking::Auth::Queues::IQueuedEntry> &NetworkEntry,
    const FString &BeaconName,
    bool bIsBeacon)
{
    using namespace ::Redpoint::EOS::Config;
    using namespace ::Redpoint::EOS::Core::Id;
    using namespace ::Redpoint::EOS::Networking;
    using namespace ::Redpoint::EOS::Networking::Auth;
    using namespace ::Redpoint::EOS::Networking::Auth::Beacon;
    using namespace ::Redpoint::EOS::Networking::Auth::Login;
    using namespace ::Redpoint::EOS::Networking::Auth::Verification;

    checkf(IsValid(Connection), TEXT("Connection must be valid when StartAuthentication is called!"));

    ENetDriverRole Role = FNetworkHelpers::GetRole(Connection);
    checkf(
        Role == ENetDriverRole::DedicatedServer || Role == ENetDriverRole::ListenServer,
        TEXT("URedpointEOSControlChannel::StartAuthentication can only be called on servers!"));

    if (!IsProductUserId(IncomingUser))
    {
        // Determine if network authentication is turned off. If it is, then we just passthrough the
        // connection.
        bool bNetworkAuthenticationEnabled = true;
        auto Driver = Cast<URedpointEOSNetDriver>(Connection->Driver);
        if (IsValid(Driver))
        {
            auto PlatformHandle = Driver->GetImpl()->GetPlatformHandle();
            if (!PlatformHandle.IsSet() ||
                PlatformHandle.GetValue()->GetEnvironment()->Config->GetNetworkAuthenticationMode() ==
                    ENetworkAuthenticationMode::Off)
            {
                bNetworkAuthenticationEnabled = false;
            }
        }

        if (!bNetworkAuthenticationEnabled)
        {
            NetworkEntry->SendSuccess();
            return;
        }

#if WITH_EDITOR
        UE_LOG(
            LogRedpointEOSNetworking,
            Warning,
            TEXT("The connecting user has not signed into EOS. Use the \"Login before play-in-editor\" option in the "
                 "toolbar if you want to have players sign into EOS before connecting to the game server. Permitting "
                 "this connection "
                 "only because you're running in the editor."));
        NetworkEntry->SendSuccess();
        return;
#else
        UE_LOG(LogRedpointEOSNetworking, Error, TEXT("Missing user ID during connection."));
        NetworkEntry->SendFailure(TEXT("Missing user ID during connection."));
        return;
#endif
    }

    if (NetworkEntry->Contains())
    {
        // We're already processing this entry.
        return;
    }

    TSharedPtr<IAuthPhaseContext> Context;
    TFunction<void()> PhaseSetupAndStart;
    if (!bIsBeacon)
    {
        TSharedRef<FAuthLoginPhaseContext> LoginContext = MakeShared<FAuthLoginPhaseContext>(this, IncomingUser);
        NetworkEntry->SetContext(StaticCastSharedRef<IAuthPhaseContext>(LoginContext));
        NetworkEntry->Track();
        Context = LoginContext;

        PhaseSetupAndStart = [this, LoginContext, NetworkEntry]() {
            const IConfig *Config;
            ENetDriverRole PhaseRole;
            if (!LoginContext->GetConfig(Config) || !LoginContext->GetRole(PhaseRole))
            {
                // Connection has gone away.
                // @todo: Is this correct? We haven't called Start yet...
                LoginContext->Finish(EAuthPhaseFailureCode::All_ConnectionHasGoneAway);
                return;
            }

            TArray<TSharedRef<IAuthLoginPhase>> Phases;
            if (Config->GetEnableAntiCheat())
            {
                Phases.Add(MakeShared<FAntiCheatProofPhase>());
                Phases.Add(MakeShared<FAntiCheatIntegrityPhase>());
            }
            LoginContext->OnCompleted().AddUObject(
                this,
                &URedpointEOSControlChannel::OnAuthenticationComplete,
                NetworkEntry);
            LoginContext->Start(LoginContext, Phases);
        };
    }
    else
    {
        TSharedRef<FAuthBeaconPhaseContext> BeaconContext =
            MakeShared<FAuthBeaconPhaseContext>(this, IncomingUser, BeaconName);
        NetworkEntry->SetContext(StaticCastSharedRef<IAuthPhaseContext>(BeaconContext));
        NetworkEntry->Track();
        Context = BeaconContext;

        PhaseSetupAndStart = [this, BeaconContext, NetworkEntry]() {
            TArray<TSharedRef<IAuthBeaconPhase>> Phases;
            // There are no beacon-specific phases at this time.
            BeaconContext->OnCompleted().AddUObject(
                this,
                &URedpointEOSControlChannel::OnAuthenticationComplete,
                NetworkEntry);
            BeaconContext->Start(BeaconContext, Phases);
        };
    }

    if (this->VerificationDatabase.Contains(IncomingUser) &&
        this->VerificationDatabase[IncomingUser] != EUserVerificationStatus::Failed)
    {
        if (this->VerificationDatabase[IncomingUser] == EUserVerificationStatus::Verified)
        {
            // We've already verified this connection is permitted to login
            // as this user, immediately start the login phases.
            PhaseSetupAndStart();
        }
        else
        {
            this->AuthVerificationContexts[IncomingUser]->OnCompleted().AddWeakLambda(
                this,
                [Context, PhaseSetupAndStart](EAuthPhaseFailureCode FailureCode, const FString &ErrorMessage) {
                    const IConfig *Config;
                    UNetConnection *Connection;
                    if (!Context->GetConfig(Config) || !Context->GetConnection(Connection))
                    {
                        // Connection has gone away.
                        Context->Finish(EAuthPhaseFailureCode::All_ConnectionHasGoneAway);
                        return;
                    }
                    if (FailureCode == EAuthPhaseFailureCode::Success)
                    {
                        if (Config->GetNetworkAuthenticationMode() != ENetworkAuthenticationMode::Off)
                        {
                            // Ensure the connection is associated with a player ID before
                            // continuing.
                            checkf(
                                Connection->PlayerId.IsValid(),
                                TEXT("Verification phases must set the player ID of the connection."));
                        }

                        // Verification passed, continue login phases.
                        PhaseSetupAndStart();
                    }
                    else
                    {
                        // Verification failed, finish now.
                        Context->Finish(FailureCode);
                    }
                });
        }
    }
    else
    {
        // We don't have a verification phase running. Start one and tell it to forward to us when it's done.
        TSharedRef<FAuthVerificationPhaseContext> VerificationContext =
            MakeShared<FAuthVerificationPhaseContext>(this, IncomingUser);

        this->VerificationDatabase.Add(IncomingUser, EUserVerificationStatus::NotStarted);
        this->AuthVerificationContexts.Add(IncomingUser, VerificationContext);

        const IConfig *Config;
        verifyf(
            Context->GetConfig(Config),
            TEXT("GetConfig should always succeed inside StartAuthentication, as the connection should not have been "
                 "closed yet!"));

        TArray<TSharedRef<IAuthVerificationPhase>> Phases;

        if (Config->GetNetworkAuthenticationMode() == ENetworkAuthenticationMode::IDToken)
        {
            if (Role == ENetDriverRole::DedicatedServer)
            {
                Phases.Add(MakeShared<FIdTokenAuthPhase>());
            }
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
            else if (Role == ENetDriverRole::ListenServer)
            {
                Phases.Add(MakeShared<FP2PAddressCheckPhase>());
            }
#endif
            else
            {
                checkf(
                    false,
                    TEXT("Network authentication mode was ID Token, but the role of the networking driver was neither "
                         "DedicatedServer nor ListenServer (was %d)."),
                    static_cast<int>(Role));
            }
        }

        // Configuration calls below already evaluate the network authentication
        // mode and adjust themselves appropriately for the networking mode.
        if (Config->GetNetworkAuthenticationMode() != ENetworkAuthenticationMode::Off)
        {
            Phases.Add(MakeShared<FSanctionCheckPhase>());
        }

        VerificationContext->OnCompleted().AddWeakLambda(
            this,
            [Context, PhaseSetupAndStart](EAuthPhaseFailureCode FailureCode, const FString &ErrorMessage) {
                const IConfig *OnCompleteConfig;
                UNetConnection *Connection;
                if (!Context->GetConfig(OnCompleteConfig) || !Context->GetConnection(Connection))
                {
                    // Connection has gone away.
                    Context->Finish(EAuthPhaseFailureCode::All_ConnectionHasGoneAway);
                    return;
                }
                if (FailureCode == EAuthPhaseFailureCode::Success)
                {
                    if (OnCompleteConfig->GetNetworkAuthenticationMode() != ENetworkAuthenticationMode::Off)
                    {
                        // Ensure the connection is associated with a player ID before
                        // continuing.
                        checkf(
                            Connection->PlayerId.IsValid(),
                            TEXT("Verification phases must set the player ID of the connection."));
                    }

                    // Verification passed, continue login phases.
                    PhaseSetupAndStart();
                }
                else
                {
                    // Verification failed, finish now.
                    Context->Finish(FailureCode);
                }
            });
        VerificationContext->Start(VerificationContext, Phases);
    }
}

void URedpointEOSControlChannel::OnAuthenticationComplete(
    ::Redpoint::EOS::Networking::Auth::EAuthPhaseFailureCode Result,
    const FString &ErrorMessage,
    TSharedRef<::Redpoint::EOS::Networking::Auth::Queues::IQueuedEntry> NetworkEntry)
{
    using namespace ::Redpoint::EOS::Networking::Auth;

    if (Result == EAuthPhaseFailureCode::Success)
    {
        NetworkEntry->SendSuccess();
    }
    else
    {
        NetworkEntry->SendFailure(ErrorMessage);
    }
    NetworkEntry->Release();
}

void URedpointEOSControlChannel::On_NMT_Login(
    const FString &ClientResponse,
    const FString &URLString,
    const UE::Online::FAccountId &IncomingUser,
    const FString &OnlinePlatformNameString)
{
    using namespace ::Redpoint::EOS::Networking::Auth::Queues;

    TSharedRef<FQueuedLoginEntry> Entry =
        MakeShared<FQueuedLoginEntry>(ClientResponse, URLString, IncomingUser, OnlinePlatformNameString, this);
    StartAuthentication(IncomingUser, StaticCastSharedRef<IQueuedEntry>(Entry), TEXT(""), false);
}

void URedpointEOSControlChannel::On_NMT_BeaconJoin(
    const FString &BeaconType,
    const UE::Online::FAccountId &IncomingUser)
{
    using namespace ::Redpoint::EOS::Networking::Auth::Queues;

    TSharedRef<FQueuedBeaconEntry> Entry = MakeShared<FQueuedBeaconEntry>(BeaconType, IncomingUser, this);
    StartAuthentication(IncomingUser, StaticCastSharedRef<IQueuedEntry>(Entry), BeaconType, true);
}

void URedpointEOSControlChannel::OnClientMustDisconnectFromServer(
    ::Redpoint::EOS::Networking::Auth::EAuthPhaseFailureCode FailureCode,
    const FString &ErrorMessage)
{
    using namespace ::Redpoint::EOS::Networking;

    UE_LOG(
        LogRedpointEOSNetworking,
        Verbose,
        TEXT("Client detected that it must disconnect from the server due to the following error: '%s'"),
        *ErrorMessage);

    if (!IsValid(Connection))
    {
        UE_LOG(
            LogRedpointEOSNetworking,
            Error,
            TEXT("Client can not handle OnClientMustDisconnectFromServer because the underlying connection has gone "
                 "away."));
        return;
    }

    if (IsValid(Connection->Driver) && Connection->Driver->Notify != nullptr)
    {
        // Emulate the server sending us an NMT_Failure.
        UE_LOG(
            LogRedpointEOSNetworking,
            Verbose,
            TEXT("Client is emulating an NMT_Failure to notify the game code of the failure reason."));
        FControlChannelOutBunch OutBunch(Connection->Channels[0], false);
        FString ErrorMessageTemp = ErrorMessage;
        OutBunch << ErrorMessageTemp;
        FInBunch InBunch(Connection, OutBunch.GetData(), OutBunch.GetNumBits());
        Connection->Driver->Notify->NotifyControlMessage(Connection, NMT_Failure, InBunch);
    }
    else
    {
        UE_LOG(
            LogRedpointEOSNetworking,
            Warning,
            TEXT("Client is hard closing the connection because the network notify interface is no longer valid."));
        Connection->Close();
    }
}

void URedpointEOSControlChannel::Init(UNetConnection *InConnection, int32 InChIndex, EChannelCreateFlags CreateFlags)
{
    using namespace ::Redpoint::EOS::AntiCheat;
    using namespace ::Redpoint::EOS::Networking::Auth;

    this->bClientTrustsServer = false;
    this->bRegisteredForAntiCheat.Reset();
    this->VerificationDatabase.Reset();
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    this->CachedEACSourceUserId.Reset();
#endif
    this->AuthStatusChangedHandle.Reset();
    this->ActionRequiredHandle.Reset();

    UControlChannel::Init(InConnection, InChIndex, CreateFlags);

    TSharedPtr<::Redpoint::EOS::API::FPlatformInstance> PlatformHandle;
    TSharedPtr<FAntiCheatSession> AntiCheatSession;
    bool bIsOwnedByBeacon;
    FNetworkHelpers::GetAntiCheat(InConnection, PlatformHandle, AntiCheatSession, bIsOwnedByBeacon);

    if (!bIsOwnedByBeacon && PlatformHandle.IsValid())
    {
        this->AuthStatusChangedHandle =
            PlatformHandle->GetSystem<IAntiCheatSystem>()->OnPlayerAuthStatusChanged().AddUObject(
                this,
                &URedpointEOSControlChannel::OnAntiCheatPlayerAuthStatusChanged);
        this->ActionRequiredHandle = PlatformHandle->GetSystem<IAntiCheatSystem>()->OnPlayerActionRequired().AddUObject(
            this,
            &URedpointEOSControlChannel::OnAntiCheatPlayerActionRequired);
    }
}

bool URedpointEOSControlChannel::CleanUp(const bool bForDestroy, EChannelCloseReason CloseReason)
{
    using namespace ::Redpoint::EOS::AntiCheat;
    using namespace ::Redpoint::EOS::Networking;
    using namespace ::Redpoint::EOS::Networking::Auth;

    // note: CleanUp gets called from garbage collection; it's not valid to try and use
    // phases or contexts here because they use TSoftObjectPtr.
    TSharedPtr<::Redpoint::EOS::API::FPlatformInstance> PlatformHandle;
    TSharedPtr<FAntiCheatSession> AntiCheatSession;
    bool bIsOwnedByBeacon;
    FNetworkHelpers::GetAntiCheat(Connection, PlatformHandle, AntiCheatSession, bIsOwnedByBeacon);
    if (!bIsOwnedByBeacon)
    {
        for (const auto &KV : this->bRegisteredForAntiCheat)
        {
            if (KV.Value)
            {
                if (PlatformHandle.IsValid() && AntiCheatSession.IsValid())
                {
                    if (!PlatformHandle->GetSystem<IAntiCheatSystem>()->UnregisterPlayer(*AntiCheatSession, KV.Key))
                    {
                        UE_LOG(LogRedpointEOSNetworking, Warning, TEXT("Anti-Cheat UnregisterPlayer call failed!"));
                    }
                }
                else
                {
                    UE_LOG(
                        LogRedpointEOSNetworking,
                        Warning,
                        TEXT("Unable to unregister player because Anti-Cheat or Anti-Cheat session is no longer "
                             "valid!"));
                }
            }
        }
    }

    return UControlChannel::CleanUp(bForDestroy, CloseReason);
}

/**
 * This implementation is copied from UControlChannel::ReceivedBunch, because we can't meaningfully intercept just
 * the NMT_Login message without copying all of the logic.
 */
void URedpointEOSControlChannel::ReceivedBunch(FInBunch &Bunch)
{
    using namespace ::Redpoint::EOS::Core::Id;
    using namespace ::Redpoint::EOS::Networking;
    using namespace ::Redpoint::EOS::Networking::Auth;
#if REDPOINT_EOS_UE_5_1_OR_LATER
    using namespace ::UE::Net;
#endif

    check(!Closing);

    // If this is a new client connection inspect the raw packet for endianess
    if (Connection && bNeedsEndianInspection && !CheckEndianess(Bunch))
    {
        // Send close bunch and shutdown this connection
        UE_LOG(
            LogNet,
            Warning,
            TEXT("UControlChannel::ReceivedBunch: NetConnection::Close() [%s] [%s] [%s] from CheckEndianess(). "
                 "FAILED. "
                 "Closing connection."),
            Connection->Driver ? *Connection->Driver->NetDriverName.ToString() : TEXT("NULL"),
            Connection->PlayerController ? *Connection->PlayerController->GetName() : TEXT("NoPC"),
            Connection->OwningActor ? *Connection->OwningActor->GetName() : TEXT("No Owner"));

        Connection->Close();
        return;
    }

    // Process the packet
    while (!Bunch.AtEnd() && Connection != nullptr &&
           Connection->GetConnectionState() !=
               USOCK_Closed) // if the connection got closed, we don't care about the rest
    {
        uint8 MessageType = 0;
        Bunch << MessageType;
        if (Bunch.IsError())
        {
            break;
        }
        int32 Pos = (int32)Bunch.GetPosBits();

        /*
        UE_NET_TRACE_DYNAMIC_NAME_SCOPE(
            FNetControlMessageInfo::GetName(MessageType),
            Bunch,
            Connection ? Connection->GetInTraceCollector() : nullptr,
            ENetTraceVerbosity::Trace);
        */

        // we handle Actor channel failure notifications ourselves
        if (MessageType == NMT_ActorChannelFailure)
        {
            if (Connection->Driver->ServerConnection == nullptr)
            {
                int32 ChannelIndex;

                if (FNetControlMessage<NMT_ActorChannelFailure>::Receive(Bunch, ChannelIndex))
                {
                    UE_LOG(
                        LogNet,
                        Log,
                        TEXT("Server connection received: %s %d %s"),
                        FNetControlMessageInfo::GetName(MessageType),
                        ChannelIndex,
                        *Describe());

                    // Check if Channel index provided by client is valid and within range of channel on server
                    if (ChannelIndex >= 0 && ChannelIndex < Connection->Channels.Num())
                    {
                        // Get the actor channel that the client provided as having failed
                        UActorChannel *ActorChan = Cast<UActorChannel>(Connection->Channels[ChannelIndex]);

                        UE_CLOG(
                            ActorChan != nullptr,
                            LogNet,
                            Log,
                            TEXT("Actor channel failed: %s"),
                            *ActorChan->Describe());

                        // The channel and the actor attached to the channel exists on the server
                        if (ActorChan != nullptr && ActorChan->Actor != nullptr)
                        {
                            // The channel that failed is the player controller thus the connection is broken
                            if (ActorChan->Actor == Connection->PlayerController)
                            {
                                UE_LOG(
                                    LogNet,
                                    Warning,
                                    TEXT("UControlChannel::ReceivedBunch: NetConnection::Close() [%s] [%s] [%s] "
                                         "from "
                                         "failed to initialize the PlayerController channel. Closing connection."),
                                    Connection->Driver ? *Connection->Driver->NetDriverName.ToString() : TEXT("NULL"),
                                    Connection->PlayerController ? *Connection->PlayerController->GetName()
                                                                 : TEXT("NoPC"),
                                    Connection->OwningActor ? *Connection->OwningActor->GetName() : TEXT("No Owner"));

                                Connection->Close();
                            }
                            // The client has a PlayerController connection, report the actor failure to
                            // PlayerController
                            else if (Connection->PlayerController != nullptr)
                            {
                                Connection->PlayerController->NotifyActorChannelFailure(ActorChan);
                            }
                            // The PlayerController connection doesn't exist for the client
                            // but the client is reporting an actor channel failure that isn't the PlayerController
                            else
                            {
                                // UE_LOG(LogNet, Warning, TEXT("UControlChannel::ReceivedBunch: PlayerController
                                // doesn't exist for the client, but the client is reporting an actor channel
                                // failure that isn't the PlayerController."));
                            }
                        }
                    }
                    // The client is sending an actor channel failure message with an invalid
                    // actor channel index
                    // @PotentialDOSAttackDetection
                    else
                    {
                        UE_LOG(
                            LogNet,
                            Warning,
                            TEXT("UControlChannel::RecievedBunch: The client is sending an actor channel failure "
                                 "message with an invalid actor channel index."));
                    }
                }
            }
        }
        else if (MessageType == NMT_GameSpecific)
        {
            // the most common Notify handlers do not support subclasses by default and so we redirect the game
            // specific messaging to the GameInstance instead
            uint8 MessageByte;
            FString MessageStr;
            if (FNetControlMessage<NMT_GameSpecific>::Receive(Bunch, MessageByte, MessageStr))
            {
                if (Connection->Driver->World != nullptr && Connection->Driver->World->GetGameInstance() != nullptr)
                {
                    Connection->Driver->World->GetGameInstance()->HandleGameNetControlMessage(
                        Connection,
                        MessageByte,
                        MessageStr);
                }
                else
                {
                    FWorldContext *Context = GEngine->GetWorldContextFromPendingNetGameNetDriver(Connection->Driver);
                    if (Context != nullptr && Context->OwningGameInstance != nullptr)
                    {
                        Context->OwningGameInstance->HandleGameNetControlMessage(Connection, MessageByte, MessageStr);
                    }
                }
            }
        }
        else if (MessageType == NMT_SecurityViolation)
        {
            FString DebugMessage;
            if (FNetControlMessage<NMT_SecurityViolation>::Receive(Bunch, DebugMessage))
            {
                UE_LOG(
                    LogSecurity,
                    Warning,
                    TEXT("%s: Closed: %s"),
                    *Connection->RemoteAddressToString(),
                    *DebugMessage);
                break;
            }
        }
#if REDPOINT_EOS_UE_5_1_OR_LATER
        else if (MessageType == NMT_DestructionInfo)
        {
            if (!Connection->Driver->IsServer())
            {
                ReceiveDestructionInfo(Bunch);
            }
            else
            {
                UE_LOG(
                    LogNet,
                    Warning,
                    TEXT("UControlChannel::ReceivedBunch: Server received a NMT_DestructionInfo which is not "
                         "supported. Closing connection."));
                Connection->Close(ENetCloseResult::ClientSentDestructionInfo);
            }
        }
        else if (MessageType == NMT_CloseReason)
        {
            FString CloseReasonList;

            if (FNetControlMessage<NMT_CloseReason>::Receive(Bunch, CloseReasonList) && !CloseReasonList.IsEmpty())
            {
                Connection->HandleReceiveCloseReason(CloseReasonList);
            }

            break;
        }
        else if (MessageType == NMT_NetPing)
        {
            ENetPingControlMessage NetPingMessageType;
            FString MessageStr;

            if (FNetControlMessage<NMT_NetPing>::Receive(Bunch, NetPingMessageType, MessageStr) &&
                NetPingMessageType <= ENetPingControlMessage::Max)
            {
                UE::Net::FNetPing::HandleNetPingControlMessage(Connection, NetPingMessageType, MessageStr);
            }

            break;
        }
#if REDPOINT_EOS_UE_5_5_OR_LATER
        else if (MessageType == NMT_IrisProtocolMismatch)
        {
            uint64 NetRefHandleId = 0;
            if (FNetControlMessage<NMT_IrisProtocolMismatch>::Receive(Bunch, NetRefHandleId))
            {
#if UE_WITH_IRIS
                if (UReplicationSystem *IrisRepSystem = Connection->Driver->GetReplicationSystem())
                {
                    IrisRepSystem->ReportProtocolMismatch(
                        NetRefHandleId,
                        Connection->GetConnectionHandle().GetParentConnectionId());
                }
#endif
            }
        }
        else if (MessageType == NMT_IrisNetRefHandleError)
        {
            UE::Net::ENetRefHandleError NetRefHandleError = UE::Net::ENetRefHandleError::None;
            uint64 NetRefHandleId = 0;
            if (FNetControlMessage<NMT_IrisNetRefHandleError>::Receive(Bunch, NetRefHandleError, NetRefHandleId))
            {
#if UE_WITH_IRIS
                if (UReplicationSystem *IrisRepSystem = Connection->Driver->GetReplicationSystem())
                {
                    IrisRepSystem->ReportErrorWithNetRefHandle(
                        NetRefHandleError,
                        NetRefHandleId,
                        Connection->GetConnectionHandle().GetParentConnectionId());
                }
#endif
            }
        }
#endif
#if REDPOINT_EOS_UE_5_6_OR_LATER
        else if (MessageType == NMT_IrisNetRefHandleErrorWithArray)
        {
            UE::Net::ENetRefHandleError NetRefHandleError = UE::Net::ENetRefHandleError::None;
            uint64 NetRefHandleId = 0;
            TArray<uint64> ExtraNetRefs;
            if (FNetControlMessage<NMT_IrisNetRefHandleErrorWithArray>::Receive(
                    Bunch,
                    NetRefHandleError,
                    NetRefHandleId,
                    ExtraNetRefs))
            {
#if UE_WITH_IRIS
                if (UReplicationSystem *IrisRepSystem = Connection->Driver->GetReplicationSystem())
                {
                    IrisRepSystem->ReportErrorWithNetRefHandle(
                        NetRefHandleError,
                        NetRefHandleId,
                        Connection->GetConnectionHandle().GetParentConnectionId(),
                        ExtraNetRefs);
                }
#endif
            }
        }
#endif
#else
        else if (MessageType == NMT_DestructionInfo)
        {
            ReceiveDestructionInfo(Bunch);
        }
        else if (MessageType == NMT_CloseReason)
        {
            FString CloseReasonList;

            if (FNetControlMessage<NMT_CloseReason>::Receive(Bunch, CloseReasonList) && !CloseReasonList.IsEmpty())
            {
                Connection->HandleReceiveCloseReason(CloseReasonList);
            }

            break;
        }
#endif
        else if (
            MessageType == NMT_EncryptionAck && FNetworkHelpers::GetConfig(this->Connection) != nullptr &&
            FNetworkHelpers::GetConfig(this->Connection)->GetEnableAutomaticEncryptionOnTrustedDedicatedServers() &&
            FNetworkHelpers::GetRole(this->Connection) == ENetDriverRole::ClientConnectedToDedicatedServer)
        {
            // This packet gets sent by servers when they call EnableEncryptionServer. We manually call
            // EnableEncryptionServer in our negotiation, but we don't use NMT_EncryptionAck (which will trigger
            // callbacks we're not handling).
            //
            // This packet will get discarded below, instead of being forwarded to the notify.
        }
        else if (
            MessageType == NMT_Failure && this->AuthConnectionContext.IsValid() && IsValid(Connection) &&
            FNetworkHelpers::GetRole(Connection) == ENetDriverRole::DedicatedServer)
        {
            // Handle NMT_Failure messages sent from clients (normally this is only sent by servers to clients, but
            // we use it to indicate errors during negotiation).
            FString ClientError;
            if (FNetControlMessage<NMT_Failure>::Receive(Bunch, ClientError))
            {
                UE_LOG(LogNet, Error, TEXT("NMT_Failure received from client: %s"), *ClientError);
            }
            else
            {
                UE_LOG(LogNet, Error, TEXT("NMT_Failure received from client (could not decode)"));
            }
            this->Connection->Close();
        }
        else if (MessageType == NMT_Hello)
        {
            // Intercept NMT_Hello to perform connection-based authentication if needed.
            uint8 IsLittleEndian;
            uint32 RemoteNetworkVersion;
            FString EncryptionToken;
#if REDPOINT_EOS_UE_5_1_OR_LATER
            EEngineNetworkRuntimeFeatures RemoteNetworkFeatures = EEngineNetworkRuntimeFeatures::None;
            if (FNetControlMessage<NMT_Hello>::Receive(
                    Bunch,
                    IsLittleEndian,
                    RemoteNetworkVersion,
                    EncryptionToken,
                    RemoteNetworkFeatures))
            {
                this->On_NMT_Hello(FOriginalParameters_NMT_Hello{
                    IsLittleEndian,
                    RemoteNetworkVersion,
                    EncryptionToken,
                    RemoteNetworkFeatures});
            }
#else
            if (FNetControlMessage<NMT_Hello>::Receive(Bunch, IsLittleEndian, RemoteNetworkVersion, EncryptionToken))
            {
                this->On_NMT_Hello(
                    FOriginalParameters_NMT_Hello{IsLittleEndian, RemoteNetworkVersion, EncryptionToken});
            }
#endif
        }
        else if (MessageType == NMT_Login)
        {
            FString ClientResponse;
            FString URLString;
            FUniqueNetIdRepl IncomingUser;
            FString OnlinePlatformNameString;
            if (FNetControlMessage<NMT_Login>::Receive(
                    Bunch,
                    ClientResponse,
                    URLString,
                    IncomingUser,
                    OnlinePlatformNameString))
            {
                this->On_NMT_Login(ClientResponse, URLString, GetAccountId(IncomingUser), OnlinePlatformNameString);
            }
        }
        else if (MessageType == NMT_BeaconJoin)
        {
            FString BeaconType;
            FUniqueNetIdRepl IncomingUser;
            if (FNetControlMessage<NMT_BeaconJoin>::Receive(Bunch, BeaconType, IncomingUser))
            {
                this->On_NMT_BeaconJoin(BeaconType, GetAccountId(IncomingUser));
            }
        }
        else if (MessageType == NMT_EOS_AntiCheatMessage)
        {
            FUniqueNetIdRepl SourceUserId;
            FUniqueNetIdRepl TargetUserId;
            TArray<uint8> AntiCheatData;
            if (FNetControlMessage<NMT_EOS_AntiCheatMessage>::Receive(Bunch, SourceUserId, TargetUserId, AntiCheatData))
            {
                this->On_NMT_EOS_AntiCheatMessage(
                    GetAccountId(SourceUserId),
                    GetAccountId(TargetUserId),
                    AntiCheatData);
            }
        }
        else if (Routes.Contains(MessageType))
        {
            if (!Routes[MessageType].IsBound() || !Routes[MessageType].Execute(this, Bunch))
            {
                UE_LOG(
                    LogNet,
                    Error,
                    TEXT("Unhandled EOS-specific control message %d on role %d. The registered router could not route "
                         "or decode the packet successfully. Closing connection."),
                    MessageType,
                    FNetworkHelpers::GetRole(Connection));
                Connection->Close();
            }
        }
        else if (MessageType >= EOS_CONTROL_CHANNEL_MESSAGE_MIN && MessageType <= EOS_CONTROL_CHANNEL_MESSAGE_MAX)
        {
            UE_LOG(
                LogNet,
                Error,
                TEXT("Unexpected EOS-specific control message %d on role %d, closing connection."),
                MessageType,
                FNetworkHelpers::GetRole(Connection));
            Connection->Close();
        }
        else if (Connection->Driver->Notify != nullptr)
        {
            // Process control message on client/server connection
            Connection->Driver->Notify->NotifyControlMessage(Connection, MessageType, Bunch);
        }

        // if the message was not handled, eat it ourselves
        if (Pos == Bunch.GetPosBits() && !Bunch.IsError())
        {
#define REDPOINT_EOS_DISCARD_MESSAGE_TYPE(Type)                                                                        \
    case NMT_##Type:                                                                                                   \
        FNetControlMessage<NMT_##Type>::Discard(Bunch);                                                                \
        break;
            switch (MessageType)
            {
                // Our message type discards, as defined in EOSControlMessages.h
                REDPOINT_EOS_DISCARD_MESSAGE_TYPE(EOS_RequestClientEphemeralKey)
                REDPOINT_EOS_DISCARD_MESSAGE_TYPE(EOS_DeliverClientEphemeralKey)
                REDPOINT_EOS_DISCARD_MESSAGE_TYPE(EOS_SymmetricKeyExchange)
                REDPOINT_EOS_DISCARD_MESSAGE_TYPE(EOS_RequestClientToken)
                REDPOINT_EOS_DISCARD_MESSAGE_TYPE(EOS_DeliverClientToken)
                REDPOINT_EOS_DISCARD_MESSAGE_TYPE(EOS_RequestTrustedClientProof)
                REDPOINT_EOS_DISCARD_MESSAGE_TYPE(EOS_DeliverTrustedClientProof)
                REDPOINT_EOS_DISCARD_MESSAGE_TYPE(EOS_AntiCheatMessage)
                REDPOINT_EOS_DISCARD_MESSAGE_TYPE(EOS_RequestIdToken)
                REDPOINT_EOS_DISCARD_MESSAGE_TYPE(EOS_DeliverIdToken)
                REDPOINT_EOS_DISCARD_MESSAGE_TYPE(EOS_EnableEncryption)
                REDPOINT_EOS_DISCARD_MESSAGE_TYPE(EOS_AcceptedMutualTrustedClientProof)

            // Engine standard message discards.
            case NMT_Hello:
                FNetControlMessage<NMT_Hello>::Discard(Bunch);
                break;
            case NMT_Welcome:
                FNetControlMessage<NMT_Welcome>::Discard(Bunch);
                break;
            case NMT_Upgrade:
                FNetControlMessage<NMT_Upgrade>::Discard(Bunch);
                break;
            case NMT_Challenge:
                FNetControlMessage<NMT_Challenge>::Discard(Bunch);
                break;
            case NMT_Netspeed:
                FNetControlMessage<NMT_Netspeed>::Discard(Bunch);
                break;
            case NMT_Login:
                FNetControlMessage<NMT_Login>::Discard(Bunch);
                break;
            case NMT_Failure:
                FNetControlMessage<NMT_Failure>::Discard(Bunch);
                break;
            case NMT_Join:
                // FNetControlMessage<NMT_Join>::Discard(Bunch);
                break;
            case NMT_JoinSplit:
                FNetControlMessage<NMT_JoinSplit>::Discard(Bunch);
                break;
            case NMT_Skip:
                FNetControlMessage<NMT_Skip>::Discard(Bunch);
                break;
            case NMT_Abort:
                FNetControlMessage<NMT_Abort>::Discard(Bunch);
                break;
            case NMT_PCSwap:
                FNetControlMessage<NMT_PCSwap>::Discard(Bunch);
                break;
            case NMT_ActorChannelFailure:
                FNetControlMessage<NMT_ActorChannelFailure>::Discard(Bunch);
                break;
            case NMT_DebugText:
                FNetControlMessage<NMT_DebugText>::Discard(Bunch);
                break;
            case NMT_NetGUIDAssign:
                FNetControlMessage<NMT_NetGUIDAssign>::Discard(Bunch);
                break;
            case NMT_EncryptionAck:
                // FNetControlMessage<NMT_EncryptionAck>::Discard(Bunch);
                break;
            case NMT_CloseReason:
                FNetControlMessage<NMT_CloseReason>::Discard(Bunch);
                break;
#if REDPOINT_EOS_UE_5_1_OR_LATER
            case NMT_NetPing:
                FNetControlMessage<NMT_NetPing>::Discard(Bunch);
                break;
#endif
            case NMT_BeaconWelcome:
                // FNetControlMessage<NMT_BeaconWelcome>::Discard(Bunch);
                break;
            case NMT_BeaconJoin:
                FNetControlMessage<NMT_BeaconJoin>::Discard(Bunch);
                break;
            case NMT_BeaconAssignGUID:
                FNetControlMessage<NMT_BeaconAssignGUID>::Discard(Bunch);
                break;
            case NMT_BeaconNetGUIDAck:
                FNetControlMessage<NMT_BeaconNetGUIDAck>::Discard(Bunch);
                break;
#if REDPOINT_EOS_UE_5_5_OR_LATER
            case NMT_IrisProtocolMismatch:
                FNetControlMessage<NMT_IrisProtocolMismatch>::Discard(Bunch);
                break;
            case NMT_IrisNetRefHandleError:
                FNetControlMessage<NMT_IrisNetRefHandleError>::Discard(Bunch);
                break;
#endif
#if REDPOINT_EOS_UE_5_6_OR_LATER
            case NMT_IrisNetRefHandleErrorWithArray:
                FNetControlMessage<NMT_IrisNetRefHandleErrorWithArray>::Discard(Bunch);
                break;
            case NMT_JoinNoPawn:
                FNetControlMessage<NMT_JoinNoPawn>::Discard(Bunch);
                break;
            case NMT_JoinNoPawnSplit:
                FNetControlMessage<NMT_JoinNoPawnSplit>::Discard(Bunch);
                break;
#endif
            default:
                // if this fails, a case is missing above for an implemented message type
                // or the connection is being sent potentially malformed packets
                // @PotentialDOSAttackDetection
                check(!FNetControlMessageInfo::IsRegistered(MessageType));

                UE_LOG(
                    LogNet,
                    Log,
                    TEXT("Received unknown control channel message %i. Closing connection."),
                    int32(MessageType));
                Connection->Close();
                return;
            }
#undef REDPOINT_EOS_DISCARD_MESSAGE_TYPE
        }
        if (Bunch.IsError())
        {
            UE_LOG(
                LogNet,
                Error,
                TEXT("Failed to read control channel message '%s'"),
                FNetControlMessageInfo::GetName(MessageType));
            break;
        }
    }

    if (Bunch.IsError())
    {
        UE_LOG(LogNet, Error, TEXT("UControlChannel::ReceivedBunch: Failed to read control channel message"));

        if (Connection != nullptr)
        {
            Connection->Close();
        }
    }
}

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()