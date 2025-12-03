// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSIdentity/IdentityUserSlot.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1245715952, Redpoint::EOS::Identity::Requests)
{
using namespace ::Redpoint::EOS::Identity;

class REDPOINTEOSIDENTITY_API FLoginRequest
{
public:
    /**
     * Initialize a login request with no authentication data.
     */
    FLoginRequest(FIdentityUserSlot InUserSlot);

    /**
     * Initialize a login request with custom authentication data.
     */
    FLoginRequest(FIdentityUserSlot InUserSlot, const FString &InType, const FString &InId, const FString &InToken);

    /**
     * The user slot to sign into.
     */
    const FIdentityUserSlot UserSlot;

    /**
     * The type of custom authentication.
     */
    const FString Type;

    /**
     * The ID for custom authentication data.
     */
    const FString Id;

    /**
     * The token for custom authentication data.
     */
    const FString Token;
};

}

namespace Redpoint::EOS::Identity::Requests
{
REDPOINT_EOS_FILE_NS_EXPORT(1245715952, Redpoint::EOS::Identity::Requests, FLoginRequest)
}

REDPOINT_EOS_CODE_GUARD_END()