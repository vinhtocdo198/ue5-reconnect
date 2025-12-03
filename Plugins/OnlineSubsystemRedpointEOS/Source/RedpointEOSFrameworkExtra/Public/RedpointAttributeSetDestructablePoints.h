// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointAttributeSetBase.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointAttributeSetDestructablePoints.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

UCLASS(BlueprintType, Category = "Redpoint EOS Online Framework|Gameplay Ability System|Attribute Sets")
class REDPOINTEOSFRAMEWORKEXTRA_API URedpointAttributeSetDestructablePoints : public URedpointAttributeSetBase
{
    GENERATED_BODY()

public:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;

    /**
     * This is distributed from the total damage done to the character. That is, if someone damages it 50% from it's
     * total health, then it recovers, and then you do damage it 100% from it's total health, you get 66% of the points
     * and the other person gets 33%.
     */
    UPROPERTY(
        BlueprintReadOnly,
        ReplicatedUsing = OnRep_TotalPointsForDestruction,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Attributes",
        Meta = (DisplayName = "Total points for destruction (distributed)"))
    FGameplayAttributeData TotalPointsForDestruction;
    REDPOINT_FRAMEWORK_ATTRIBUTE_ACCESSORS(URedpointAttributeSetDestructablePoints, TotalPointsForDestruction)

    // Unknown attribute - find usage or remove.
    UPROPERTY(
        BlueprintReadOnly,
        ReplicatedUsing = OnRep_FinalHitPointsForDestruction,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Attributes",
        Meta = (DisplayName = "Final points for destruction"))
    FGameplayAttributeData FinalHitPointsForDestruction;
    REDPOINT_FRAMEWORK_ATTRIBUTE_ACCESSORS(URedpointAttributeSetDestructablePoints, FinalHitPointsForDestruction)

    // Unknown attribute - find usage or remove.
    UPROPERTY(
        BlueprintReadOnly,
        ReplicatedUsing = OnRep_TotalPointsDroppedOnDestruction,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Attributes",
        Meta = (DisplayName = "Total points dropped on destruction"))
    FGameplayAttributeData TotalPointsDroppedOnDestruction;
    REDPOINT_FRAMEWORK_ATTRIBUTE_ACCESSORS(URedpointAttributeSetDestructablePoints, TotalPointsDroppedOnDestruction)

    // Unknown attribute - find usage or remove.
    UPROPERTY(
        BlueprintReadOnly,
        ReplicatedUsing = OnRep_PointsForRecoveryToExitAfterStun,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Attributes",
        Meta = (DisplayName = "Points for recovery to exit after stun"))
    FGameplayAttributeData PointsForRecoveryToExitAfterStun;
    REDPOINT_FRAMEWORK_ATTRIBUTE_ACCESSORS(URedpointAttributeSetDestructablePoints, PointsForRecoveryToExitAfterStun)

protected:
    UFUNCTION()
    virtual void OnRep_TotalPointsForDestruction(const FGameplayAttributeData &OldValue);

    UFUNCTION()
    virtual void OnRep_FinalHitPointsForDestruction(const FGameplayAttributeData &OldValue);

    UFUNCTION()
    virtual void OnRep_TotalPointsDroppedOnDestruction(const FGameplayAttributeData &OldValue);

    UFUNCTION()
    virtual void OnRep_PointsForRecoveryToExitAfterStun(const FGameplayAttributeData &OldValue);
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()