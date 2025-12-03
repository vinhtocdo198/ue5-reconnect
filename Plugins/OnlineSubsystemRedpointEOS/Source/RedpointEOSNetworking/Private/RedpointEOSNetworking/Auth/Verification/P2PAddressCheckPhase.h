// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "RedpointEOSNetworking/Auth/Verification/AuthVerificationPhase.h"
#include "RedpointEOSNetworking/Auth/Verification/AuthVerificationPhaseContext.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(4036443453, Redpoint::EOS::Networking::Auth::Verification)
{
using namespace ::Redpoint::EOS::Networking::Auth::Verification;

extern const FName AuthPhaseP2PAddressCheck;

class FP2PAddressCheckPhase : public IAuthVerificationPhase, public TSharedFromThis<FP2PAddressCheckPhase>
{
public:
    FP2PAddressCheckPhase() = default;
    UE_NONCOPYABLE(FP2PAddressCheckPhase);
    virtual ~FP2PAddressCheckPhase() override = default;

    virtual FName GetName() const override;

    static void RegisterRoutes(URedpointEOSControlChannel *ControlChannel);
    virtual void Start(const TSharedRef<FAuthVerificationPhaseContext> &Context) override;

private:
    void On_NMT_EOS_RequestClientToken(const TSharedRef<FAuthVerificationPhaseContext> &Context);
    void On_NMT_EOS_DeliverClientToken(
        const TSharedRef<FAuthVerificationPhaseContext> &Context,
        const FString &ClientTokenType,
        const FString &ClientDisplayName,
        const FString &ClientToken);
};

}

namespace Redpoint::EOS::Networking::Auth::Verification
{
REDPOINT_EOS_FILE_NS_EXPORT(4036443453, Redpoint::EOS::Networking::Auth::Verification, FP2PAddressCheckPhase)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()