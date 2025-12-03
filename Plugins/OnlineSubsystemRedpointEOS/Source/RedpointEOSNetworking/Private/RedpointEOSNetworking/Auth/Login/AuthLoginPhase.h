// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSNetworking/Auth/AuthPhase.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(233779596, Redpoint::EOS::Networking::Auth::Login, FAuthLoginPhaseContext);

namespace REDPOINT_EOS_FILE_NS_ID(238432179, Redpoint::EOS::Networking::Auth::Login)
{
using namespace ::Redpoint::EOS::Networking::Auth;
using namespace ::Redpoint::EOS::Networking::Auth::Login;

class IAuthLoginPhase : public IAuthPhase
{
public:
    /**
     * Begin the phase. This is always only called on the server, which ultimately
     * controls the authentication sequence.
     */
    virtual void Start(const TSharedRef<FAuthLoginPhaseContext> &Context) = 0;

    /**
     * Handle Anti-Cheat authentication status change. This is optional to implement.
     */
    virtual void OnAntiCheatPlayerAuthStatusChanged(
        const TSharedRef<FAuthLoginPhaseContext> &Context,
        EOS_EAntiCheatCommonClientAuthStatus NewAuthStatus){};

    /**
     * Handle Anti-Cheat action required. This is optional to implement.
     */
    virtual void OnAntiCheatPlayerActionRequired(
        const TSharedRef<FAuthLoginPhaseContext> &Context,
        EOS_EAntiCheatCommonClientAction ClientAction,
        EOS_EAntiCheatCommonClientActionReason ActionReasonCode,
        const FString &ActionReasonDetailsString){};
};

}

namespace Redpoint::EOS::Networking::Auth::Login
{
REDPOINT_EOS_FILE_NS_EXPORT(238432179, Redpoint::EOS::Networking::Auth::Login, IAuthLoginPhase)
}

REDPOINT_EOS_CODE_GUARD_END()