// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSControlChannel.h"
#include "RedpointEOSNetworking/Auth/Verification/AuthVerificationPhase.h"
#include "RedpointEOSNetworking/Auth/Verification/AuthVerificationPhaseContext.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(366962924, Redpoint::EOS::Networking::Auth::Verification)
{
using namespace ::Redpoint::EOS::Networking::Auth::Verification;

extern const FName AuthPhaseIdTokenAuth;

class FIdTokenAuthPhase : public IAuthVerificationPhase, public TSharedFromThis<FIdTokenAuthPhase>
{
public:
    FIdTokenAuthPhase() = default;
    UE_NONCOPYABLE(FIdTokenAuthPhase);
    virtual ~FIdTokenAuthPhase() override = default;

    virtual FName GetName() const override;

    static void RegisterRoutes(URedpointEOSControlChannel *ControlChannel);
    virtual void Start(const TSharedRef<FAuthVerificationPhaseContext> &Context) override;

private:
    void On_NMT_EOS_RequestIdToken(const TSharedRef<FAuthVerificationPhaseContext> &Context);
    void On_NMT_EOS_DeliverIdToken(
        const TSharedRef<FAuthVerificationPhaseContext> &Context,
        const FString &ClientToken);
};

}

namespace Redpoint::EOS::Networking::Auth::Verification
{
REDPOINT_EOS_FILE_NS_EXPORT(366962924, Redpoint::EOS::Networking::Auth::Verification, FIdTokenAuthPhase)
}

REDPOINT_EOS_CODE_GUARD_END()