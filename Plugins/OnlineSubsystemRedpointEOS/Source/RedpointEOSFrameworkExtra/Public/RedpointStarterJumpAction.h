// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_FLOAT_CONVERSION_PERMIT_BEGIN()

#include "InputAction.h"

#include "RedpointStarterJumpAction.generated.h"

/**
 * A digital (bool) input action that is used for jumping in the starter player controller when no other input action
 * has been specified.
 */
UCLASS(BlueprintType)
class REDPOINTEOSFRAMEWORKEXTRA_API URedpointStarterJumpAction : public UInputAction
{
    GENERATED_BODY()

public:
    URedpointStarterJumpAction();
};

REDPOINT_EOS_FLOAT_CONVERSION_PERMIT_END()