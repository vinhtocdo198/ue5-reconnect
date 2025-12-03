// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Templates::Util
{

enum class EAllocationFlags : uint8_t
{
    // Default allocation behaviour.
    Default,

    // Return nullptr instead of allocating an empty string/array if the
    // original value is empty.
    ReturnNullptrIfEmpty,
};

}

REDPOINT_EOS_CODE_GUARD_END()