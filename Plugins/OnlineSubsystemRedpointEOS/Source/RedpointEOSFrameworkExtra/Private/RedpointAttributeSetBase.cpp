// Copyright June Rhodes. All Rights Reserved.

#include "RedpointAttributeSetBase.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

void URedpointAttributeSetBase::AdjustAttributeForMaxChange(
    FGameplayAttributeData &AffectedAttribute,
    const FGameplayAttributeData &MaxAttribute,
    float NewMaxValue,
    const FGameplayAttribute &AffectedAttributeProperty)
{
    UAbilitySystemComponent *AbilityComp = GetOwningAbilitySystemComponent();

    const float CurrentMaxValue = MaxAttribute.GetCurrentValue();

    if (!FMath::IsNearlyEqual(CurrentMaxValue, NewMaxValue) && AbilityComp)
    {
        // Change current value to maintain the current Val / Max percent
        const float CurrentValue = AffectedAttribute.GetCurrentValue();
        float NewDelta =
            (CurrentMaxValue > 0.f) ? (CurrentValue * NewMaxValue / CurrentMaxValue) - CurrentValue : NewMaxValue;

        AbilityComp->ApplyModToAttributeUnsafe(AffectedAttributeProperty, EGameplayModOp::Additive, NewDelta);
    }
}

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()