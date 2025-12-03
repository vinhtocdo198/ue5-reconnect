// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSNetworking/Auth/Verification/AuthVerificationPhase.h"
#include "RedpointEOSNetworking/Auth/Verification/AuthVerificationPhaseContext.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(29325491, Redpoint::EOS::Networking::Auth::Verification)
{
using namespace ::Redpoint::EOS::Networking::Auth::Verification;

extern const FName AuthPhaseSanctionCheck;

class FSanctionCheckPhase : public IAuthVerificationPhase, public TSharedFromThis<FSanctionCheckPhase>
{
public:
    FSanctionCheckPhase() = default;
    UE_NONCOPYABLE(FSanctionCheckPhase);
    virtual ~FSanctionCheckPhase() override = default;

    virtual FName GetName() const override;

    static void RegisterRoutes(URedpointEOSControlChannel *ControlChannel);
    virtual void Start(const TSharedRef<FAuthVerificationPhaseContext> &Context) override;
};

}

namespace Redpoint::EOS::Networking::Auth::Verification
{
REDPOINT_EOS_FILE_NS_EXPORT(29325491, Redpoint::EOS::Networking::Auth::Verification, FSanctionCheckPhase)
}

REDPOINT_EOS_CODE_GUARD_END()