// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3705160141, Redpoint::EOS::Identity)
{

/**
 * Represents a local "slot" that can be signed into. Slots are typically associated with controllers on console
 * platforms, but can also be virtual slots for automation testing scenarios where there's no real local user.
 */
typedef int32 FIdentityUserSlot;

}

namespace Redpoint::EOS::Identity
{
REDPOINT_EOS_FILE_NS_EXPORT(3705160141, Redpoint::EOS::Identity, FIdentityUserSlot)
}

REDPOINT_EOS_CODE_GUARD_END()