// Copyright June Rhodes. All Rights Reserved.

#include "RedpointAbilityCharacterMovementComponent.h"

#include "RedpointOverridesMaxSpeedInterface.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

float URedpointAbilityCharacterMovementComponent::GetMaxSpeed() const
{
    if (this->GetOwner()->Implements<URedpointOverridesMaxSpeedInterface>())
    {
        // Delegate to the owner's GetMaxSpeed implementation, which in turn can delegate
        // to the ability system component.
        return IRedpointOverridesMaxSpeedInterface::Execute_GetMaxSpeed(this->GetOwner());
    }

    return Super::GetMaxSpeed();
}

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()