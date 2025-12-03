// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSNetworking/Auth/AuthPhaseContext.h"
#include "RedpointEOSNetworking/Auth/UserVerificationStatus.h"
#include "RedpointEOSNetworking/Auth/Verification/AuthVerificationPhase.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1384477030, Redpoint::EOS::Networking::Auth::Verification)
{
using namespace ::Redpoint::EOS::Networking::Auth;
using namespace ::Redpoint::EOS::Networking::Auth::Verification;

class FAuthVerificationPhaseContext : public FAuthPhaseContext<IAuthVerificationPhase, FAuthVerificationPhaseContext>,
                                      public TSharedFromThis<FAuthVerificationPhaseContext>
{
private:
    UE::Online::FAccountId UserId;

protected:
    virtual FString GetIdentifier() const override;
    virtual FString GetPhaseGroup() const override;

public:
    FAuthVerificationPhaseContext(URedpointEOSControlChannel *InControlChannel, const UE::Online::FAccountId &InUserId);
    UE_NONCOPYABLE(FAuthVerificationPhaseContext);
    virtual ~FAuthVerificationPhaseContext() override = default;

    void SetVerificationStatus(EUserVerificationStatus InStatus);
    bool GetVerificationStatus(EUserVerificationStatus &OutStatus) const;

    /** Returns if the client trusts the dedicated server. Only used for legacy authentication. */
    bool IsConnectionAsTrustedOnClient(bool &OutIsTrusted) const;

    UE::Online::FAccountId GetUserId() const;
};

}

namespace Redpoint::EOS::Networking::Auth::Verification
{
REDPOINT_EOS_FILE_NS_EXPORT(1384477030, Redpoint::EOS::Networking::Auth::Verification, FAuthVerificationPhaseContext)
}

REDPOINT_EOS_CODE_GUARD_END()