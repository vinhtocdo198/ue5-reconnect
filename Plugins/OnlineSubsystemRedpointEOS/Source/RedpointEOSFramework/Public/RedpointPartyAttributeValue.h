// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointPartyAttributeType.h"

#include "RedpointPartyAttributeValue.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

/**
 * Represents a party attribute value or party member attribute value.
 */
USTRUCT(
    BlueprintType,
    meta = (Category = "Redpoint EOS Online Framework", DisplayName = "Redpoint Party Attribute Value"))
struct REDPOINTEOSFRAMEWORK_API FRedpointPartyAttributeValue
{
    GENERATED_BODY()

public:
    // The type of value stored in this attribute.
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Redpoint EOS Online Framework")
    ERedpointPartyAttributeType Type = ERedpointPartyAttributeType::Bool;

    // The string value, if the type is set to 'String'.
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Redpoint EOS Online Framework")
    FString String;

    // The 64-bit integer value value, if the type is set to 'Int64'.
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Redpoint EOS Online Framework")
    int64 Int64 = 0;

    // The double precision floating point value value, if the type is set to 'Double'.
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Redpoint EOS Online Framework")
    double Double = 0.0;

    // The boolean value, if the type is set to 'Bool'.
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Redpoint EOS Online Framework")
    bool Bool = false;
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()