// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "GameFramework/CharacterMovementComponent.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointAbilityCharacterMovementComponent.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

UCLASS()
class REDPOINTEOSFRAMEWORKEXTRA_API URedpointAbilityCharacterMovementComponent : public UCharacterMovementComponent
{
    GENERATED_BODY()

public:
    virtual float GetMaxSpeed() const override;
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()