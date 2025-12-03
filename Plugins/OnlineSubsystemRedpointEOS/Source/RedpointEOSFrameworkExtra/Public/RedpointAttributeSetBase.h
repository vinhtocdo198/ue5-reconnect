// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointAttributeSetBase.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

#define REDPOINT_FRAMEWORK_ATTRIBUTE_ACCESSORS(ClassName, PropertyName)                                                \
    GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName)                                                         \
    GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName)                                                                       \
    GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName)                                                                       \
    GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * The base class for Redpoint attribute sets, providing a function that adjusts an attribute value in response to a
 * "maximum" attribute value changing. For example, if maximum health goes below current health, this function can be
 * used to ensure that current health is adjusted to equal the new maximum health value.
 */
UCLASS(BlueprintType, Abstract, Category = "Redpoint EOS Online Framework|Gameplay Ability System|Attribute Sets")
class REDPOINTEOSFRAMEWORKEXTRA_API URedpointAttributeSetBase : public UAttributeSet
{
    GENERATED_BODY()

protected:
    void AdjustAttributeForMaxChange(
        FGameplayAttributeData &AffectedAttribute,
        const FGameplayAttributeData &MaxAttribute,
        float NewMaxValue,
        const FGameplayAttribute &AffectedAttributeProperty);
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()