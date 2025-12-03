// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSControlChannel.h"
#include "RedpointEOSNetworking/Auth/Login/AuthLoginPhase.h"
#include "RedpointEOSNetworking/Auth/Login/AuthLoginPhaseContext.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1089039527, Redpoint::EOS::Networking::Auth::Login)
{
using namespace ::Redpoint::EOS::AntiCheat;
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Config;
using namespace ::Redpoint::EOS::Networking::Auth::Login;

extern const FName AuthPhaseAntiCheatIntegrity;

class FAntiCheatIntegrityPhase : public IAuthLoginPhase
{
private:
    bool bIsStarted;
    bool bIsVerified;

public:
    FAntiCheatIntegrityPhase();
    UE_NONCOPYABLE(FAntiCheatIntegrityPhase);
    virtual ~FAntiCheatIntegrityPhase() override = default;

    virtual FName GetName() const override;

    static void RegisterRoutes(URedpointEOSControlChannel *ControlChannel);
    virtual void Start(const TSharedRef<FAuthLoginPhaseContext> &Context) override;

    virtual void OnAntiCheatPlayerAuthStatusChanged(
        const TSharedRef<FAuthLoginPhaseContext> &Context,
        EOS_EAntiCheatCommonClientAuthStatus NewAuthStatus) override;
    virtual void OnAntiCheatPlayerActionRequired(
        const TSharedRef<FAuthLoginPhaseContext> &Context,
        EOS_EAntiCheatCommonClientAction ClientAction,
        EOS_EAntiCheatCommonClientActionReason ActionReasonCode,
        const FString &ActionReasonDetailsString) override;
};

}

namespace Redpoint::EOS::Networking::Auth::Login
{
REDPOINT_EOS_FILE_NS_EXPORT(1089039527, Redpoint::EOS::Networking::Auth::Login, FAntiCheatIntegrityPhase)
}

REDPOINT_EOS_CODE_GUARD_END()