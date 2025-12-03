// Copyright June Rhodes. All Rights Reserved.

#include "RedpointAbilityBaseGameplayAbility.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

bool URedpointAbilityBaseGameplayAbility::CheckCost(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo *ActorInfo,
    OUT FGameplayTagContainer *OptionalRelevantTags) const
{
    return Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags) && this->K2_CheckCost(Handle, *ActorInfo);
}

void URedpointAbilityBaseGameplayAbility::ApplyCost(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo *ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo) const
{
    this->K2_ApplyCost(Handle, *ActorInfo, ActivationInfo);
    Super::ApplyCost(Handle, ActorInfo, ActivationInfo);
}

bool URedpointAbilityBaseGameplayAbility::K2_CheckCost_Implementation(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo &ActorInfo) const
{
    return true;
}

void URedpointAbilityBaseGameplayAbility::K2_ApplyCost_Implementation(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo &ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo) const
{
}

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()