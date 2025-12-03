// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Abilities/GameplayAbility.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointAbilityBaseGameplayAbility.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

/**
 * This is an enhanced version of UGameplayAbility that exposes CheckCost and ApplyCost to blueprints. It should be used
 * when you need to check data on an item that a gameplay ability is triggering from (such as a weapon being fired),
 * where that data is not coming from an attribute set.
 *
 * If you don't need to override CheckCost or ApplyCost, you should use UGameplayAbility directly.
 */
UCLASS(
    BlueprintType,
    Category = "Redpoint EOS Online Framework|Gameplay Ability System|Abilities",
    meta = (DisplayName = "Redpoint Ability Base Gameplay Ability"))
class REDPOINTEOSFRAMEWORKEXTRA_API URedpointAbilityBaseGameplayAbility : public UGameplayAbility
{
    GENERATED_BODY()

public:
    virtual bool CheckCost(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo *ActorInfo,
        OUT FGameplayTagContainer *OptionalRelevantTags = nullptr) const override;

    virtual void ApplyCost(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo *ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo) const override;

    UFUNCTION(
        BlueprintNativeEvent,
        BlueprintCallable,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Abilities",
        meta = (DisplayName = "Check Cost"))
    bool K2_CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo &ActorInfo) const;
    virtual bool K2_CheckCost_Implementation(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo &ActorInfo) const;

    UFUNCTION(
        BlueprintNativeEvent,
        BlueprintCallable,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Abilities",
        meta = (DisplayName = "Apply Cost"))
    void K2_ApplyCost(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo &ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo) const;
    virtual void K2_ApplyCost_Implementation(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo &ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo) const;
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()