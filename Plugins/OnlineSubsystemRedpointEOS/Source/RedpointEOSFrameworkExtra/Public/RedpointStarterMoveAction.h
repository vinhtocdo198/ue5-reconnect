// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_FLOAT_CONVERSION_PERMIT_BEGIN()

#include "InputAction.h"

#include "RedpointStarterMoveAction.generated.h"

/**
 * A 2D vector input action that is used for movement in the starter player controller when no other input action has
 * been specified.
 */
UCLASS(BlueprintType)
class REDPOINTEOSFRAMEWORKEXTRA_API URedpointStarterMoveAction : public UInputAction
{
    GENERATED_BODY()

public:
    URedpointStarterMoveAction();
};

REDPOINT_EOS_FLOAT_CONVERSION_PERMIT_END()