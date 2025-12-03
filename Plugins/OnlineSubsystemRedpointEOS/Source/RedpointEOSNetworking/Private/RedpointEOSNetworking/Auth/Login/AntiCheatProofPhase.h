// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSAntiCheat/AntiCheatImplementationType.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSControlChannel.h"
#include "RedpointEOSNetworking/Auth/Login/AuthLoginPhase.h"
#include "RedpointEOSNetworking/Auth/Login/AuthLoginPhaseContext.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1843523477, Redpoint::EOS::Networking::Auth::Login)
{
using namespace ::Redpoint::EOS::AntiCheat;
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Config;
using namespace ::Redpoint::EOS::Networking::Auth::Login;

extern const FName AuthPhaseAntiCheatProof;

class FAntiCheatProofPhase : public IAuthLoginPhase
{
private:
    FString PendingNonceCheck;
    bool bReceivedRemoteAuthForListenServerFromClient;
    bool bWaitingForClientToAcceptOurProof;

    void InitializeNonceForRequestProof();
    void RequestProofFromPeer(
        const TSharedRef<FAuthLoginPhaseContext> &Context,
        const TSharedPtr<FPlatformInstance> &PlatformHandle,
        UNetConnection *Connection,
        const FUniqueNetIdRepl &UserIdRepl);
    void DeliverProofToPeer(
        const TSharedRef<FAuthLoginPhaseContext> &Context,
        const TSharedPtr<FPlatformInstance> &PlatformHandle,
        const IConfig *Config,
        UNetConnection *Connection,
        const FString &EncodedNonce,
        bool bRequestingTrustedClientProofFromListenServer);

public:
    FAntiCheatProofPhase();
    UE_NONCOPYABLE(FAntiCheatProofPhase);
    virtual ~FAntiCheatProofPhase() override = default;

    virtual FName GetName() const override;

    static void RegisterRoutes(URedpointEOSControlChannel *ControlChannel);
    virtual void Start(const TSharedRef<FAuthLoginPhaseContext> &Context) override;

    virtual void OnAntiCheatPlayerAuthStatusChanged(
        const TSharedRef<FAuthLoginPhaseContext> &Context,
        EOS_EAntiCheatCommonClientAuthStatus NewAuthStatus) override;

private:
    void On_NMT_EOS_RequestTrustedClientProof(
        const TSharedRef<FAuthLoginPhaseContext> &Context,
        const FString &EncodedNonce,
        EAntiCheatImplementationType AntiCheatImplementationType);
    void On_NMT_EOS_DeliverTrustedClientProof(
        const TSharedRef<FAuthLoginPhaseContext> &Context,
        bool bCanProvideProof,
        const FString &EncodedProof,
        const FString &PlatformString,
        bool bRequestMutualProofFromListenServer,
        FString EncodedNonceForListenServer,
        bool bSkipPeerRegistration);
    void On_NMT_EOS_AcceptedMutualTrustedClientProof(const TSharedRef<FAuthLoginPhaseContext> &Context);
};

}

namespace Redpoint::EOS::Networking::Auth::Login
{
REDPOINT_EOS_FILE_NS_EXPORT(1843523477, Redpoint::EOS::Networking::Auth::Login, FAntiCheatProofPhase)
}

REDPOINT_EOS_CODE_GUARD_END()