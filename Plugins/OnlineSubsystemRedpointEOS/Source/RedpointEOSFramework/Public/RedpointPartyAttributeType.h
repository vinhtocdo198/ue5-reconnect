// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointPartyAttributeType.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

/**
 * Indicates the type of value stored in the party attribute.
 */
UENUM(BlueprintType, meta = (Category = "Redpoint EOS Online Framework", DisplayName = "Redpoint Party Attribute Type"))
enum class ERedpointPartyAttributeType : uint8
{
    // The value is a string value.
    String,

    // The value is a 64-bit integer value.
    Int64,

    // The value is a double precision floating point value.
    Double,

    // The value is a boolean value.
    Bool,
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()