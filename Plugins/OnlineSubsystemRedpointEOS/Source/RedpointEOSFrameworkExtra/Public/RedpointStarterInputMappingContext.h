// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_FLOAT_CONVERSION_PERMIT_BEGIN()

#include "InputMappingContext.h"

#include "RedpointStarterInputMappingContext.generated.h"

/**
 * An input mapping context that is used by the starter player controller when no other input mapping context is
 * specified.
 */
UCLASS(BlueprintType)
class REDPOINTEOSFRAMEWORKEXTRA_API URedpointStarterInputMappingContext : public UInputMappingContext
{
    GENERATED_BODY()

public:
    URedpointStarterInputMappingContext();
};

REDPOINT_EOS_FLOAT_CONVERSION_PERMIT_END()