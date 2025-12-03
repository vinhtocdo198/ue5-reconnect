// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSNetworking/Auth/AuthPhase.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(
    1384477030,
    Redpoint::EOS::Networking::Auth::Verification,
    FAuthVerificationPhaseContext);

namespace REDPOINT_EOS_FILE_NS_ID(632213569, Redpoint::EOS::Networking::Auth::Verification)
{
using namespace ::Redpoint::EOS::Networking::Auth;
using namespace ::Redpoint::EOS::Networking::Auth::Verification;

class IAuthVerificationPhase : public IAuthPhase
{
public:
    /**
     * Begin the phase. This is always only called on the server, which ultimately
     * controls the authentication sequence.
     */
    virtual void Start(const TSharedRef<FAuthVerificationPhaseContext> &Context) = 0;
};

}

namespace Redpoint::EOS::Networking::Auth::Verification
{
REDPOINT_EOS_FILE_NS_EXPORT(632213569, Redpoint::EOS::Networking::Auth::Verification, IAuthVerificationPhase)
}

REDPOINT_EOS_CODE_GUARD_END()