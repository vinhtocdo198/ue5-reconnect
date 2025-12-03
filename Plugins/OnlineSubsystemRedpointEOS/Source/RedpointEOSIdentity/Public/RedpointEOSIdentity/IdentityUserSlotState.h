// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(733542834, Redpoint::EOS::Identity)
{
using namespace ::Redpoint::EOS::Identity;

/**
 * Whether or not a local slot is currently signed in.
 */
enum EIdentityUserSlotState
{
    NotSignedIn,

    SignedIn,
};

}

namespace Redpoint::EOS::Identity
{
REDPOINT_EOS_FILE_NS_EXPORT(733542834, Redpoint::EOS::Identity, EIdentityUserSlotState)
}

REDPOINT_EOS_CODE_GUARD_END()