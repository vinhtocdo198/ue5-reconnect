// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSControlChannel.h"
#include "RedpointEOSNetworking/Auth/AuthPhaseContext.h"
#include "RedpointEOSNetworking/Auth/Login/AuthLoginPhase.h"
#include "RedpointEOSNetworking/Auth/UserVerificationStatus.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(233779596, Redpoint::EOS::Networking::Auth::Login)
{
using namespace ::Redpoint::EOS::Networking::Auth;
using namespace ::Redpoint::EOS::Networking::Auth::Login;

class FAuthLoginPhaseContext : public FAuthPhaseContext<IAuthLoginPhase, FAuthLoginPhaseContext>,
                               public TSharedFromThis<FAuthLoginPhaseContext>
{
private:
    UE::Online::FAccountId UserId;

protected:
    virtual FString GetIdentifier() const override;
    virtual FString GetPhaseGroup() const override;

public:
    FAuthLoginPhaseContext(URedpointEOSControlChannel *InControlChannel, const UE::Online::FAccountId &InUserId);
    UE_NONCOPYABLE(FAuthLoginPhaseContext);
    virtual ~FAuthLoginPhaseContext() override = default;

    /** Marks the user as registered for Anti-Cheat, which ensures the control channel will unregister the player when
     * the connection closes. */
    void MarkAsRegisteredForAntiCheat();

    // Used by Anti-Cheat phases, even though this should probably be removed...
    void SetVerificationStatus(EUserVerificationStatus InStatus);
    bool GetVerificationStatus(EUserVerificationStatus &OutStatus) const;

    UE::Online::FAccountId GetUserId() const;
};

}

namespace Redpoint::EOS::Networking::Auth::Login
{
REDPOINT_EOS_FILE_NS_EXPORT(233779596, Redpoint::EOS::Networking::Auth::Login, FAuthLoginPhaseContext)
}

REDPOINT_EOS_CODE_GUARD_END()