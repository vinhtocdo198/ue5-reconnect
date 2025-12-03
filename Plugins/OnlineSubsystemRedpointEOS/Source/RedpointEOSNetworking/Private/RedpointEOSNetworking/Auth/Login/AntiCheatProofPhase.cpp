// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSNetworking/Auth/Login/AntiCheatProofPhase.h"

#include "Misc/Base64.h"
#include "RedpointEOSAntiCheat/AntiCheatSystem.h"
#include "RedpointEOSControlChannel.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSCore/Module.h"
#include "RedpointEOSNetworking/Logging.h"
#include "RedpointLibHydrogen.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#define hydro_sign_NONCEBYTES 32

namespace REDPOINT_EOS_FILE_NS_ID(1843523477, Redpoint::EOS::Networking::Auth::Login)
{
using namespace ::Redpoint::EOS::AntiCheat;
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Config;
using namespace ::Redpoint::EOS::Core;
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::Networking;
using namespace ::Redpoint::EOS::Networking::Auth;

const FName AuthPhaseAntiCheatProof = FName(TEXT("AntiCheatProof"));

FAntiCheatProofPhase::FAntiCheatProofPhase()
    : PendingNonceCheck()
    , bReceivedRemoteAuthForListenServerFromClient(false)
    , bWaitingForClientToAcceptOurProof(false)
{
}

FName FAntiCheatProofPhase::GetName() const
{
    return AuthPhaseAntiCheatProof;
}

void FAntiCheatProofPhase::InitializeNonceForRequestProof()
{
    char Nonce[hydro_sign_NONCEBYTES];
    FMemory::Memzero(&Nonce, hydro_sign_NONCEBYTES);
    hydro_random_buf(&Nonce, hydro_sign_NONCEBYTES);
    checkf(this->PendingNonceCheck.IsEmpty(), TEXT("There is already an outbound proof verification in progress."));
    this->PendingNonceCheck = FBase64::Encode(reinterpret_cast<uint8 *>(&Nonce), hydro_sign_NONCEBYTES);
}

void FAntiCheatProofPhase::RequestProofFromPeer(
    const TSharedRef<FAuthLoginPhaseContext> &Context,
    const TSharedPtr<FPlatformInstance> &PlatformHandle,
    UNetConnection *Connection,
    const FUniqueNetIdRepl &UserIdRepl)
{
    // Request the trusted client proof from the peer. Certain platforms like consoles are able to run as
    // unprotected clients and still be secure, so they don't need Anti-Cheat active. To ensure that peers don't
    // pretend as if they're on console, we make peers send a cryptographic proof. Only the console builds contain the
    // private key necessary for signing the proof.
    UE_LOG(
        LogRedpointEOSNetworkAuth,
        Verbose,
        TEXT("Server authentication: %s: Requesting trusted client proof from remote peer %s..."),
        *GetUserIdString(Context->GetUserId()),
        *UserIdRepl.ToString());
    this->InitializeNonceForRequestProof();
    uint8 ImplementationType = (uint8)PlatformHandle->GetSystem<IAntiCheatSystem>()->GetImplementationType();
    FUniqueNetIdRepl UserIdReplTemp = UserIdRepl;
    FNetControlMessage<NMT_EOS_RequestTrustedClientProof>::Send(
        Connection,
        UserIdReplTemp,
        this->PendingNonceCheck,
        ImplementationType);
}

void FAntiCheatProofPhase::DeliverProofToPeer(
    const TSharedRef<FAuthLoginPhaseContext> &Context,
    const TSharedPtr<FPlatformInstance> &PlatformHandle,
    const IConfig *Config,
    UNetConnection *Connection,
    const FString &EncodedNonce,
    bool bRequestingTrustedClientProofFromListenServer)
{
    bool bCanProvideProof = false;
    FString EncodedProof = TEXT("");
    FString PlatformString = TEXT("");

    if (Config == nullptr)
    {
        UE_LOG(
            LogRedpointEOSNetworkAuth,
            Warning,
            TEXT("Can't provide client proof, as configuration is not available."));
    }
    else
    {
        FString TrustedClientPrivateKey = Config->GetTrustedClientPrivateKey();
        if (!TrustedClientPrivateKey.IsEmpty())
        {
            TArray<uint8> TrustedClientPrivateKeyBytes;
            if (FBase64::Decode(TrustedClientPrivateKey, TrustedClientPrivateKeyBytes) &&
                TrustedClientPrivateKeyBytes.Num() == hydro_sign_SECRETKEYBYTES)
            {
                TArray<uint8> DecodedNonce;
                if (FBase64::Decode(EncodedNonce, DecodedNonce) && DecodedNonce.Num() == hydro_sign_NONCEBYTES)
                {
                    uint8_t signature[hydro_sign_BYTES];
                    if (hydro_sign_create(
                            signature,
                            DecodedNonce.GetData(),
                            static_cast<size_t>(DecodedNonce.Num()),
                            "TRSTPROF",
                            TrustedClientPrivateKeyBytes.GetData()) == 0)
                    {
                        TArray<uint8> SignatureBytes(&signature[0], hydro_sign_BYTES);
                        EncodedProof = FBase64::Encode(SignatureBytes);
                        bCanProvideProof = true;

                        PlatformString = FModule::GetModuleChecked().GetRuntimePlatform()->GetAntiCheatPlatformName();
                    }
                }
            }
        }
    }

    if (bCanProvideProof)
    {
        UE_LOG(LogRedpointEOSNetworkAuth, Verbose, TEXT("Providing trusted client proof..."));
    }
    else
    {
        UE_LOG(
            LogRedpointEOSNetworkAuth,
            Verbose,
            TEXT("Responding to trusted client proof request with no proof available..."));
    }

    if (bRequestingTrustedClientProofFromListenServer)
    {
        this->InitializeNonceForRequestProof();
    }

    auto UserIdRepl = GetUniqueNetIdRepl(Context->GetUserId());
    bool bSkipPeerRegistration =
        bCanProvideProof && PlatformHandle->GetSystem<IAntiCheatSystem>()->ShouldRemoteSkipPeerRegistration();
    FNetControlMessage<NMT_EOS_DeliverTrustedClientProof>::Send(
        Connection,
        UserIdRepl,
        bCanProvideProof,
        EncodedProof,
        PlatformString,
        bRequestingTrustedClientProofFromListenServer,
        this->PendingNonceCheck,
        bSkipPeerRegistration);
}

void FAntiCheatProofPhase::RegisterRoutes(URedpointEOSControlChannel *CC)
{
    CC->AddRoute(
        NMT_EOS_RequestTrustedClientProof,
        FAuthPhaseRoute::CreateLambda([](URedpointEOSControlChannel *ControlChannel, FInBunch &Bunch) {
            FUniqueNetIdRepl TargetUserId;
            FString EncodedNonce;
            uint8 AntiCheatImplementationType;
            if (FNetControlMessage<NMT_EOS_RequestTrustedClientProof>::Receive(
                    Bunch,
                    TargetUserId,
                    EncodedNonce,
                    AntiCheatImplementationType))
            {
                TSharedPtr<FAuthLoginPhaseContext> Context = ControlChannel->GetAuthLoginPhaseContext(TargetUserId);
                if (Context)
                {
                    TSharedPtr<FAntiCheatProofPhase> Phase =
                        Context->GetPhase<FAntiCheatProofPhase>(AuthPhaseAntiCheatProof);
                    if (Phase)
                    {
                        Phase->On_NMT_EOS_RequestTrustedClientProof(
                            Context.ToSharedRef(),
                            EncodedNonce,
                            (EAntiCheatImplementationType)AntiCheatImplementationType);
                        return true;
                    }
                }
            }
            return false;
        }));
    CC->AddRoute(
        NMT_EOS_DeliverTrustedClientProof,
        FAuthPhaseRoute::CreateLambda([](URedpointEOSControlChannel *ControlChannel, FInBunch &Bunch) {
            FUniqueNetIdRepl TargetUserId;
            bool bCanProvideProof;
            FString EncodedProof;
            FString PlatformString;
            bool bRequestMutualProofFromListenServer;
            FString EncodedNonceForListenServer;
            bool bSkipPeerRegistration;
            if (FNetControlMessage<NMT_EOS_DeliverTrustedClientProof>::Receive(
                    Bunch,
                    TargetUserId,
                    bCanProvideProof,
                    EncodedProof,
                    PlatformString,
                    bRequestMutualProofFromListenServer,
                    EncodedNonceForListenServer,
                    bSkipPeerRegistration))
            {
                TSharedPtr<FAuthLoginPhaseContext> Context = ControlChannel->GetAuthLoginPhaseContext(TargetUserId);
                if (Context)
                {
                    TSharedPtr<FAntiCheatProofPhase> Phase =
                        Context->GetPhase<FAntiCheatProofPhase>(AuthPhaseAntiCheatProof);
                    if (Phase)
                    {
                        Phase->On_NMT_EOS_DeliverTrustedClientProof(
                            Context.ToSharedRef(),
                            bCanProvideProof,
                            EncodedProof,
                            PlatformString,
                            bRequestMutualProofFromListenServer,
                            EncodedNonceForListenServer,
                            bSkipPeerRegistration);
                        return true;
                    }
                }
            }
            return false;
        }));
    CC->AddRoute(
        NMT_EOS_AcceptedMutualTrustedClientProof,
        FAuthPhaseRoute::CreateLambda([](URedpointEOSControlChannel *ControlChannel, FInBunch &Bunch) {
            FUniqueNetIdRepl TargetUserId;
            if (FNetControlMessage<NMT_EOS_AcceptedMutualTrustedClientProof>::Receive(Bunch, TargetUserId))
            {
                TSharedPtr<FAuthLoginPhaseContext> Context = ControlChannel->GetAuthLoginPhaseContext(TargetUserId);
                if (Context)
                {
                    TSharedPtr<FAntiCheatProofPhase> Phase =
                        Context->GetPhase<FAntiCheatProofPhase>(AuthPhaseAntiCheatProof);
                    if (Phase)
                    {
                        Phase->On_NMT_EOS_AcceptedMutualTrustedClientProof(Context.ToSharedRef());
                        return true;
                    }
                }
            }
            return false;
        }));
}

void FAntiCheatProofPhase::Start(const TSharedRef<FAuthLoginPhaseContext> &Context)
{
    UNetConnection *Connection;
    const IConfig *Config;
    if (!Context->GetConnection(Connection) || !Context->GetConfig(Config))
    {
        Context->Finish(EAuthPhaseFailureCode::All_ConnectionHasGoneAway);
        return;
    }

    checkf(
        Connection->PlayerId.IsValid(),
        TEXT("Connection player ID must have been set by verification phase before Anti-Cheat phases can begin."));

    Context->SetVerificationStatus(EUserVerificationStatus::EstablishingAntiCheatProof);

    TSharedPtr<FPlatformInstance> PlatformHandle;
    TSharedPtr<FAntiCheatSession> AntiCheatSession;
    bool bIsOwnedByBeacon;
    if (!Context->GetAntiCheat(PlatformHandle, AntiCheatSession, bIsOwnedByBeacon))
    {
        Context->Finish(EAuthPhaseFailureCode::All_ConnectionHasGoneAway);
        return;
    }
    if (bIsOwnedByBeacon)
    {
        // Beacons do not use Anti-Cheat, because Anti-Cheat only allows one Anti-Cheat connection
        // for the main game session.
        UE_LOG(
            LogRedpointEOSNetworkAuth,
            Verbose,
            TEXT("Server authentication: %s: Skipping Anti-Cheat connection because this is a beacon connection..."),
            *GetUserIdString(Context->GetUserId()));
        Context->Finish(EAuthPhaseFailureCode::Success);
        return;
    }
    if (!PlatformHandle.IsValid() || !AntiCheatSession.IsValid())
    {
        UE_LOG(
            LogRedpointEOSNetworkAuth,
            Error,
            TEXT("Server authentication: %s: Failed to obtain Anti-Cheat interface or session."),
            *GetUserIdString(Context->GetUserId()));
        Context->Finish(EAuthPhaseFailureCode::All_CanNotAccessAntiCheat);
        return;
    }

    auto UserIdRepl = GetUniqueNetIdRepl(Context->GetUserId());

    // Check if we can request a proof.
    if (Config == nullptr)
    {
        UE_LOG(
            LogRedpointEOSNetworkAuth,
            Error,
            TEXT("Server authentication: %s: Failed to obtain configuration."),
            *GetUserIdString(Context->GetUserId()));
        Context->Finish(EAuthPhaseFailureCode::All_CanNotAccessConfig);
        return;
    }
    FString TrustedClientPublicKey = Config->GetTrustedClientPublicKey();
    if (TrustedClientPublicKey.IsEmpty())
    {
        // We can't request proof verification from the client, because we don't have a public key.
        this->On_NMT_EOS_DeliverTrustedClientProof(Context, false, TEXT(""), TEXT(""), false, TEXT(""), false);
        return;
    }

    // Request the trusted client proof from the peer.
    this->RequestProofFromPeer(Context, PlatformHandle, Connection, UserIdRepl);
}

void FAntiCheatProofPhase::On_NMT_EOS_RequestTrustedClientProof(
    const TSharedRef<FAuthLoginPhaseContext> &Context,
    const FString &EncodedNonce,
    EAntiCheatImplementationType AntiCheatImplementationType)
{
    ENetDriverRole Role;
    if (!Context->GetRole(Role))
    {
        Context->Finish(EAuthPhaseFailureCode::All_CanNotAccessConfig);
        return;
    }

    if (Role == ENetDriverRole::DedicatedServer)
    {
        // Dedicated servers can't be asked to proof their Anti-Cheat status. We only allow
        // trusted client proofs in the following scenarios:
        //
        // - ListenServer: A player is hosting a listen server on their machine, and clients need to verify the listen
        //				   server is intact.
        // - ClientConnectedToDedicatedServer: A player is connecting to a dedicated server, and the server needs to
        //                                     verify the client is intact.
        // - ClientConnectedToListenServer: A player is connecting to a listen server, and the server needs to verify
        //                                  the client is intact.
        Context->Finish(EAuthPhaseFailureCode::Phase_AntiCheatProof_NotPermittedToRequestClientProof);
        return;
    }

    UNetConnection *Connection;
    const IConfig *Config;
    if (!Context->GetConnection(Connection) || !Context->GetConfig(Config))
    {
        Context->Finish(EAuthPhaseFailureCode::All_ConnectionHasGoneAway);
        return;
    }

    TSharedPtr<::Redpoint::EOS::API::FPlatformInstance> PlatformHandle;
    TSharedPtr<FAntiCheatSession> AntiCheatSession;
    bool bIsBeacon;
    if (Context->GetAntiCheat(PlatformHandle, AntiCheatSession, bIsBeacon) && PlatformHandle.IsValid() &&
        PlatformHandle->GetSystem<IAntiCheatSystem>()->GetImplementationType() != AntiCheatImplementationType)
    {
        Context->Finish(EAuthPhaseFailureCode::Phase_AntiCheatProof_AntiCheatImplementationNotCompatible);
        return;
    }

    // Deliver the requested proof, if possible, to the peer. If we are a client connecting to
    // a listen server, we ask the listen server to prove their EAC status or prove that they're
    // a trusted client.
    this->DeliverProofToPeer(
        Context,
        PlatformHandle,
        Config,
        Connection,
        EncodedNonce,
        Role == ENetDriverRole::ClientConnectedToListenServer);
}

void FAntiCheatProofPhase::On_NMT_EOS_DeliverTrustedClientProof(
    const TSharedRef<FAuthLoginPhaseContext> &Context,
    bool bCanProvideProof,
    const FString &EncodedProof,
    const FString &PlatformString,
    bool bRequestMutualProofFromListenServer,
    FString EncodedNonceForListenServer,
    bool bSkipPeerRegistration)
{
    ENetDriverRole Role;
    if (!Context->GetRole(Role))
    {
        Context->Finish(EAuthPhaseFailureCode::All_CanNotAccessConfig);
        return;
    }

    if (Role == ENetDriverRole::ClientConnectedToDedicatedServer)
    {
        // This is unexpected; as a client we shouldn't have been able to ask
        // a dedicated server for a trusted client proof.
        Context->Finish(EAuthPhaseFailureCode::Phase_AntiCheatProof_NotPermittedToRequestClientProof);
        return;
    }

    if (Role != ENetDriverRole::ListenServer && bRequestMutualProofFromListenServer)
    {
        // Clients can't anyone other than a listen server to do a mutual proof.
        Context->Finish(EAuthPhaseFailureCode::Phase_AntiCheatProof_NotPermittedToRequestClientProof);
        return;
    }

    UNetConnection *Connection;
    const IConfig *Config;
    if (!Context->GetConnection(Connection) || !Context->GetConfig(Config))
    {
        Context->Finish(EAuthPhaseFailureCode::All_ConnectionHasGoneAway);
        return;
    }

    auto UserId = Context->GetUserId();
    if (!IsProductUserId(UserId))
    {
        UE_LOG(LogRedpointEOSNetworkAuth, Warning, TEXT("Ignoring NMT_EOS_DeliverTrustedClientProof, missing UserId."));
        return;
    }

    EUserVerificationStatus VerificationStatus;
    if (Role != ENetDriverRole::ClientConnectedToListenServer)
    {
        if (!Context->GetVerificationStatus(VerificationStatus))
        {
            Context->Finish(EAuthPhaseFailureCode::All_ConnectionHasGoneAway);
            return;
        }
    }
    else
    {
        // When clients receive this, they assume they're establishing the Anti-Cheat proof
        // with the listen server.
        VerificationStatus = EUserVerificationStatus::EstablishingAntiCheatProof;
    }
    if (VerificationStatus != EUserVerificationStatus::EstablishingAntiCheatProof)
    {
        UE_LOG(LogRedpointEOSNetworkAuth, Warning, TEXT("Ignoring NMT_EOS_DeliverTrustedClientProof, invalid UserId."));
        return;
    }

    TSharedPtr<FPlatformInstance> PlatformHandle;
    TSharedPtr<FAntiCheatSession> AntiCheatSession;
    bool bIsOwnedByBeacon;
    if (!Context->GetAntiCheat(PlatformHandle, AntiCheatSession, bIsOwnedByBeacon))
    {
        Context->Finish(EAuthPhaseFailureCode::All_ConnectionHasGoneAway);
        return;
    }
    checkf(!bIsOwnedByBeacon, TEXT("Did not expect beacon connection to also negotiate Anti-Cheat."));
    if (!PlatformHandle.IsValid() || !AntiCheatSession.IsValid())
    {
        UE_LOG(
            LogRedpointEOSNetworkAuth,
            Error,
            TEXT("Server authentication: %s: Failed to obtain Anti-Cheat interface or session."),
            *GetUserIdString(UserId));
        Context->Finish(EAuthPhaseFailureCode::All_CanNotAccessAntiCheat);
        return;
    }

    if (Config == nullptr)
    {
        UE_LOG(
            LogRedpointEOSNetworkAuth,
            Error,
            TEXT("Server authentication: %s: Unable to get configuration when verifying client proof."),
            *GetUserIdString(UserId));
        Context->Finish(EAuthPhaseFailureCode::All_CanNotAccessConfig);
        return;
    }

    EOS_EAntiCheatCommonClientType ClientType = EOS_EAntiCheatCommonClientType::EOS_ACCCT_ProtectedClient;
    EOS_EAntiCheatCommonClientPlatform ClientPlatform = EOS_EAntiCheatCommonClientPlatform::EOS_ACCCP_Unknown;
    if (bCanProvideProof)
    {
        bool bProofValid = false;
        FString TrustedClientPublicKey = Config->GetTrustedClientPublicKey();
        if (!TrustedClientPublicKey.IsEmpty())
        {
            TArray<uint8> TrustedClientPublicKeyBytes;
            if (FBase64::Decode(TrustedClientPublicKey, TrustedClientPublicKeyBytes) &&
                TrustedClientPublicKeyBytes.Num() == hydro_sign_PUBLICKEYBYTES)
            {
                TArray<uint8> PendingNonce;
                if (!this->PendingNonceCheck.IsEmpty() && FBase64::Decode(this->PendingNonceCheck, PendingNonce) &&
                    PendingNonce.Num() == hydro_sign_NONCEBYTES)
                {
                    TArray<uint8> Signature;
                    if (FBase64::Decode(EncodedProof, Signature))
                    {
                        if (Signature.Num() == hydro_sign_BYTES)
                        {
                            if (hydro_sign_verify(
                                    Signature.GetData(),
                                    PendingNonce.GetData(),
                                    static_cast<size_t>(PendingNonce.Num()),
                                    "TRSTPROF",
                                    TrustedClientPublicKeyBytes.GetData()) == 0)
                            {
                                bProofValid = true;
                            }
                            else
                            {
                                UE_LOG(
                                    LogRedpointEOSNetworkAuth,
                                    Error,
                                    TEXT("Server authentication: %s: hydro_sign_verify failed to verify the provided "
                                         "signature."),
                                    *GetUserIdString(UserId));
                            }
                        }
                        else
                        {
                            UE_LOG(
                                LogRedpointEOSNetworkAuth,
                                Error,
                                TEXT("Server authentication: %s: Provided signature from client was not the correct "
                                     "length."),
                                *GetUserIdString(UserId));
                        }
                    }
                    else
                    {
                        UE_LOG(
                            LogRedpointEOSNetworkAuth,
                            Error,
                            TEXT("Server authentication: %s: Server could not decode provided signature."),
                            *GetUserIdString(UserId));
                    }
                }
                else
                {
                    UE_LOG(
                        LogRedpointEOSNetworkAuth,
                        Error,
                        TEXT("Server authentication: %s: Server does not have a pending nonce or it is the incorrect "
                             "length."),
                        *GetUserIdString(UserId));
                }
            }
            else
            {
                UE_LOG(
                    LogRedpointEOSNetworkAuth,
                    Error,
                    TEXT("Server authentication: %s: Server does not have public key for trusted clients."),
                    *GetUserIdString(UserId));
            }
        }
        if (!bProofValid)
        {
            UE_LOG(
                LogRedpointEOSNetworkAuth,
                Error,
                TEXT("Server authentication: %s: Invalid signature for unprotected client proof."),
                *GetUserIdString(UserId));
            Context->Finish(EAuthPhaseFailureCode::Phase_AntiCheatProof_InvalidSignatureForUnprotectedClient);
            return;
        }

        ClientType = EOS_EAntiCheatCommonClientType::EOS_ACCCT_UnprotectedClient;
        if (PlatformString == TEXT("Xbox"))
        {
            ClientPlatform = EOS_EAntiCheatCommonClientPlatform::EOS_ACCCP_Xbox;
        }
        else if (PlatformString == TEXT("PlayStation"))
        {
            ClientPlatform = EOS_EAntiCheatCommonClientPlatform::EOS_ACCCP_PlayStation;
        }
        else if (PlatformString == TEXT("Nintendo"))
        {
            ClientPlatform = EOS_EAntiCheatCommonClientPlatform::EOS_ACCCP_Nintendo;
        }
        else if (PlatformString == TEXT("Windows"))
        {
            ClientPlatform = EOS_EAntiCheatCommonClientPlatform::EOS_ACCCP_Windows;
        }
        else if (PlatformString == TEXT("Mac"))
        {
            ClientPlatform = EOS_EAntiCheatCommonClientPlatform::EOS_ACCCP_Mac;
        }
        else if (PlatformString == TEXT("Linux"))
        {
            ClientPlatform = EOS_EAntiCheatCommonClientPlatform::EOS_ACCCP_Linux;
        }
        else if (PlatformString == TEXT("Android"))
        {
            ClientPlatform = EOS_EAntiCheatCommonClientPlatform::EOS_ACCCP_Android;
        }
        else if (PlatformString == TEXT("IOS"))
        {
            ClientPlatform = EOS_EAntiCheatCommonClientPlatform::EOS_ACCCP_iOS;
        }
    }

    if (ClientType == EOS_EAntiCheatCommonClientType::EOS_ACCCT_ProtectedClient)
    {
        // Only trusted clients can skip peer registration.
        bSkipPeerRegistration = false;
    }

    UE_LOG(
        LogRedpointEOSNetworkAuth,
        Verbose,
        TEXT("Server authentication: %s: Received proof data from %s (%s)."),
        *GetUserIdString(UserId),
        Role == ENetDriverRole::ClientConnectedToListenServer ? TEXT("listen server") : TEXT("client"),
        ClientType == EOS_EAntiCheatCommonClientType::EOS_ACCCT_ProtectedClient
            ? TEXT("protected")
            : (bSkipPeerRegistration ? TEXT("skipped+trusted") : TEXT("unprotected+trusted")));

    if (Role != ENetDriverRole::ClientConnectedToListenServer)
    {
        if (ClientType == EOS_EAntiCheatCommonClientType::EOS_ACCCT_ProtectedClient)
        {
            Context->SetVerificationStatus(EUserVerificationStatus::WaitingForAntiCheatIntegrity);
        }
        else
        {
            Context->SetVerificationStatus(EUserVerificationStatus::Verified);
        }
    }

    if (!bSkipPeerRegistration && Role != ENetDriverRole::ClientConnectedToListenServer)
    {
        if (!PlatformHandle->GetSystem<IAntiCheatSystem>()
                 ->RegisterPlayer(*AntiCheatSession, UserId, ClientType, ClientPlatform))
        {
            UE_LOG(
                LogRedpointEOSNetworkAuth,
                Error,
                TEXT("Server authentication: %s: Failed to register player with Anti-Cheat."),
                *GetUserIdString(UserId));
            Context->Finish(EAuthPhaseFailureCode::Phase_AntiCheatProof_AntiCheatRegistrationFailed);
            return;
        }

        Context->MarkAsRegisteredForAntiCheat();
    }
    else if (Role == ENetDriverRole::ClientConnectedToListenServer)
    {
        UE_LOG(
            LogRedpointEOSNetworkAuth,
            Verbose,
            TEXT("Server authentication: %s: Skipping peer registration on client connected to listen server because "
                 "the listen server will already have been registered as a player when CreateSession was originally "
                 "called for Anti-Cheat."),
            *GetUserIdDebugString(UserId));
    }
    else
    {
        UE_LOG(
            LogRedpointEOSNetworkAuth,
            Verbose,
            TEXT("Server authentication: %s: Skipping peer registration because they're a trusted client and they can "
                 "not process network messages."),
            *GetUserIdString(UserId));
    }

    if (Role != ENetDriverRole::ClientConnectedToListenServer)
    {
        if (bRequestMutualProofFromListenServer)
        {
            UE_LOG(
                LogRedpointEOSNetworkAuth,
                Verbose,
                TEXT("Server authentication: %s: Registered player with Anti-Cheat. Remote client requested mutual "
                     "trusted client proof, sending proof if we have one."),
                *GetUserIdString(UserId));

            this->bWaitingForClientToAcceptOurProof = true;
            this->DeliverProofToPeer(Context, PlatformHandle, Config, Connection, EncodedNonceForListenServer, false);

            // Now we must wait for On_NMT_EOS_AcceptedMutualTrustedClientProof.
        }
        else
        {
            UE_LOG(
                LogRedpointEOSNetworkAuth,
                Verbose,
                TEXT("Server authentication: %s: Registered player with Anti-Cheat. Now waiting for Anti-Cheat "
                     "verification status."),
                *GetUserIdString(UserId));

            Context->Finish(EAuthPhaseFailureCode::Success);
        }
    }
    else
    {
        if (ClientType == EOS_EAntiCheatCommonClientType::EOS_ACCCT_ProtectedClient)
        {
            // The listen server is a protected client. Wait until OnAntiCheatPlayerAuthStatusChanged
            // fires before we send EOS_AcceptedMutualTrustedClientProof.
        }
        else
        {
            // The listen server is an unprotected trusted client. We immediately accept their proof
            // and don't wait for EAC events.
            if (!this->bReceivedRemoteAuthForListenServerFromClient)
            {
                UE_LOG(
                    LogRedpointEOSNetworkAuth,
                    Verbose,
                    TEXT("Server authentication: %s: Notifying the listen server that we now trust them (as a trusted "
                         "client)."),
                    *GetUserIdString(UserId));
                this->bReceivedRemoteAuthForListenServerFromClient = true;
                FUniqueNetIdRepl TargetUserId = GetUniqueNetIdRepl(UserId);
                FNetControlMessage<NMT_EOS_AcceptedMutualTrustedClientProof>::Send(Connection, TargetUserId);
            }
        }
    }
}

void FAntiCheatProofPhase::On_NMT_EOS_AcceptedMutualTrustedClientProof(
    const TSharedRef<FAuthLoginPhaseContext> &Context)
{
    auto UserId = Context->GetUserId();

    ENetDriverRole Role;
    if (!Context->GetRole(Role) || Role != ENetDriverRole::ListenServer)
    {
        UE_LOG(
            LogRedpointEOSNetworkAuth,
            Warning,
            TEXT("Server authentication: %s: Ignoring NMT_EOS_AcceptedMutualTrustedClientProof message because we "
                 "do not have the correct role to handle it."),
            *GetUserIdString(UserId));
        Context->Finish(EAuthPhaseFailureCode::All_InvalidMessageType);
        return;
    }
    if (!this->bWaitingForClientToAcceptOurProof)
    {
        UE_LOG(
            LogRedpointEOSNetworkAuth,
            Warning,
            TEXT("Server authentication: %s: Ignoring NMT_EOS_AcceptedMutualTrustedClientProof message because we "
                 "weren't waiting for it."),
            *GetUserIdString(UserId));
        Context->Finish(EAuthPhaseFailureCode::All_InvalidMessageType);
        return;
    }

    UE_LOG(
        LogRedpointEOSNetworkAuth,
        Verbose,
        TEXT("Server authentication: %s: Client accepted our listen server trusted client proof. Continuing with "
             "network authentication."),
        *GetUserIdString(UserId));

    Context->Finish(EAuthPhaseFailureCode::Success);
}

void FAntiCheatProofPhase::OnAntiCheatPlayerAuthStatusChanged(
    const TSharedRef<FAuthLoginPhaseContext> &Context,
    EOS_EAntiCheatCommonClientAuthStatus NewAuthStatus)
{
    ENetDriverRole Role;
    if (!Context->GetRole(Role) || Role != ENetDriverRole::ClientConnectedToListenServer)
    {
        // Just silently drop events here, in case we receive them but don't care about them.
        return;
    }
    UNetConnection *Connection;
    if (!Context->GetConnection(Connection))
    {
        Context->Finish(EAuthPhaseFailureCode::All_ConnectionHasGoneAway);
        return;
    }

    FString StatusStr = TEXT("Unknown");
    if (NewAuthStatus == EOS_EAntiCheatCommonClientAuthStatus::EOS_ACCCAS_Invalid)
    {
        StatusStr = TEXT("Invalid");
    }
    else if (NewAuthStatus == EOS_EAntiCheatCommonClientAuthStatus::EOS_ACCCAS_LocalAuthComplete)
    {
        StatusStr = TEXT("LocalAuthComplete");
    }
    else if (NewAuthStatus == EOS_EAntiCheatCommonClientAuthStatus::EOS_ACCCAS_RemoteAuthComplete)
    {
        StatusStr = TEXT("RemoteAuthComplete");
    }

    UE_LOG(
        LogRedpointEOSNetworkAuth,
        Verbose,
        TEXT("Server authentication: %s: Anti-Cheat verification status of remote listen server is now '%s'."),
        *GetUserIdString(Context->GetUserId()),
        *StatusStr);

    if (NewAuthStatus == EOS_EAntiCheatCommonClientAuthStatus::EOS_ACCCAS_RemoteAuthComplete)
    {
        if (!this->bReceivedRemoteAuthForListenServerFromClient)
        {
            UE_LOG(
                LogRedpointEOSNetworkAuth,
                Verbose,
                TEXT("Server authentication: %s: Notifying the listen server that we now trust them (as a protected "
                     "client)."),
                *GetUserIdString(Context->GetUserId()));
            this->bReceivedRemoteAuthForListenServerFromClient = true;
            FUniqueNetIdRepl TargetUserId = GetUniqueNetIdRepl(Context->GetUserId());
            FNetControlMessage<NMT_EOS_AcceptedMutualTrustedClientProof>::Send(Connection, TargetUserId);
        }
    }
}

#undef hydro_sign_NONCEBYTES

}

REDPOINT_EOS_CODE_GUARD_END()