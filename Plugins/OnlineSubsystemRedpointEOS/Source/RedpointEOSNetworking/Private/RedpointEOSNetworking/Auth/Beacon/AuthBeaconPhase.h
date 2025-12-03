// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSNetworking/Auth/AuthPhase.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(3050450103, Redpoint::EOS::Networking::Auth::Beacon, FAuthBeaconPhaseContext)

namespace REDPOINT_EOS_FILE_NS_ID(1706863799, Redpoint::EOS::Networking::Auth::Beacon)
{
using namespace ::Redpoint::EOS::Networking::Auth;
using namespace ::Redpoint::EOS::Networking::Auth::Beacon;

class IAuthBeaconPhase : public IAuthPhase
{
public:
    virtual ~IAuthBeaconPhase() override = default;

    /**
     * Begin the phase. This is always only called on the server, which ultimately
     * controls the authentication sequence.
     */
    virtual void Start(const TSharedRef<FAuthBeaconPhaseContext> &Context) = 0;
};

}

namespace Redpoint::EOS::Networking::Auth::Beacon
{
REDPOINT_EOS_FILE_NS_EXPORT(1706863799, Redpoint::EOS::Networking::Auth::Beacon, IAuthBeaconPhase)
}

REDPOINT_EOS_CODE_GUARD_END()