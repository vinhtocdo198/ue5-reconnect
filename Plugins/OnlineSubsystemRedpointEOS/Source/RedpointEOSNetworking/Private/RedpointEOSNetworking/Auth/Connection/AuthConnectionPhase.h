// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSNetworking/Auth/AuthPhase.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(
    1581436555,
    Redpoint::EOS::Networking::Auth::Connection,
    FAuthConnectionPhaseContext)

namespace REDPOINT_EOS_FILE_NS_ID(63703879, Redpoint::EOS::Networking::Auth::Connection)
{
using namespace ::Redpoint::EOS::Networking::Auth;
using namespace ::Redpoint::EOS::Networking::Auth::Connection;

class IAuthConnectionPhase : public IAuthPhase
{
public:
    virtual ~IAuthConnectionPhase() override = default;

    /**
     * Begin the phase. This is always only called on the server, which ultimately
     * controls the authentication sequence.
     */
    virtual void Start(const TSharedRef<FAuthConnectionPhaseContext> &Context) = 0;
};

}

namespace Redpoint::EOS::Networking::Auth::Connection
{
REDPOINT_EOS_FILE_NS_EXPORT(63703879, Redpoint::EOS::Networking::Auth::Connection, IAuthConnectionPhase)
}

REDPOINT_EOS_CODE_GUARD_END()