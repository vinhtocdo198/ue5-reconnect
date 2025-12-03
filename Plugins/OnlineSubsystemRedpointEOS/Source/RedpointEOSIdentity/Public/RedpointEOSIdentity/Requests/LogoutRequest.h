// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSIdentity/IdentityUserSlot.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(4051209205, Redpoint::EOS::Identity::Requests)
{
using namespace ::Redpoint::EOS::Identity;

class REDPOINTEOSIDENTITY_API FLogoutRequest
{
public:
    /**
     * Initialize a logout request.
     */
    FLogoutRequest(FIdentityUserSlot InUserSlot);

    /**
     * The user slot to sign out of.
     */
    const FIdentityUserSlot UserSlot;
};

}

namespace Redpoint::EOS::Identity::Requests
{
REDPOINT_EOS_FILE_NS_EXPORT(4051209205, Redpoint::EOS::Identity::Requests, FLogoutRequest)
}

REDPOINT_EOS_CODE_GUARD_END()