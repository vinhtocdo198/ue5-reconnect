// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Net/DataBunch.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSControlChannel.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1124735540, Redpoint::EOS::Networking::Auth)
{

class IAuthPhase
{
public:
    IAuthPhase() = default;
    UE_NONCOPYABLE(IAuthPhase);
    virtual ~IAuthPhase() = default;

    /**
     * The type of this phase. Used by decoders to route calls.
     */
    virtual FName GetName() const = 0;
};

}

namespace Redpoint::EOS::Networking::Auth
{
REDPOINT_EOS_FILE_NS_EXPORT(1124735540, Redpoint::EOS::Networking::Auth, IAuthPhase);
}

REDPOINT_EOS_CODE_GUARD_END()