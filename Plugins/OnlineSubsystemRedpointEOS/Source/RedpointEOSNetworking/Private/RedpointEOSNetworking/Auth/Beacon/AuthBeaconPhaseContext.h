// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSNetworking/Auth/AuthPhaseContext.h"
#include "RedpointEOSNetworking/Auth/Beacon/AuthBeaconPhase.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3050450103, Redpoint::EOS::Networking::Auth::Beacon)
{
using namespace ::Redpoint::EOS::Networking::Auth;
using namespace ::Redpoint::EOS::Networking::Auth::Beacon;

class FAuthBeaconPhaseContext : public FAuthPhaseContext<IAuthBeaconPhase, FAuthBeaconPhaseContext>,
                                public TSharedFromThis<FAuthBeaconPhaseContext>
{
private:
    UE::Online::FAccountId UserId;
    FString BeaconName;

protected:
    virtual FString GetIdentifier() const override;
    virtual FString GetPhaseGroup() const override;

public:
    FAuthBeaconPhaseContext(
        URedpointEOSControlChannel *InControlChannel,
        UE::Online::FAccountId InUserId,
        const FString &InBeaconName);
    UE_NONCOPYABLE(FAuthBeaconPhaseContext);
    virtual ~FAuthBeaconPhaseContext() override = default;

    UE::Online::FAccountId GetUserId() const;
    FString GetBeaconName() const;
};

}

namespace Redpoint::EOS::Networking::Auth::Beacon
{
REDPOINT_EOS_FILE_NS_EXPORT(3050450103, Redpoint::EOS::Networking::Auth::Beacon, FAuthBeaconPhaseContext)
}

REDPOINT_EOS_CODE_GUARD_END()