// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointAttributeSetBase.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointAttributeSetStun.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

UCLASS(BlueprintType, Category = "Redpoint EOS Online Framework|Gameplay Ability System|Attribute Sets")
class REDPOINTEOSFRAMEWORKEXTRA_API URedpointAttributeSetStun : public URedpointAttributeSetBase
{
    GENERATED_BODY()

public:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;

    /**
     * The amount of time until health starts increasing again. - June: Is this actually just the amount of time until
     * stun wears off?
     */
    UPROPERTY(
        BlueprintReadOnly,
        ReplicatedUsing = OnRep_StunTime,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Attributes",
        Meta = (DisplayName = "Stun Time (seconds)"))
    FGameplayAttributeData StunTime;
    REDPOINT_FRAMEWORK_ATTRIBUTE_ACCESSORS(URedpointAttributeSetStun, StunTime)

    /**
     * The value of health that the current health will be set to after the stun wears off.
     */
    UPROPERTY(
        BlueprintReadOnly,
        ReplicatedUsing = OnRep_StunRecoveryHealthAmount,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Attributes",
        Meta = (DisplayName = "Stun Recovery Health Amount"))
    FGameplayAttributeData StunRecoveryHealthAmount;
    REDPOINT_FRAMEWORK_ATTRIBUTE_ACCESSORS(URedpointAttributeSetStun, StunRecoveryHealthAmount)

protected:
    UFUNCTION()
    virtual void OnRep_StunTime(const FGameplayAttributeData &OldValue);

    UFUNCTION()
    virtual void OnRep_StunRecoveryHealthAmount(const FGameplayAttributeData &OldValue);
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()