// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "AbilitySystemComponent.h"
#include "GameFramework/Actor.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3297874782, Redpoint::EOS::Framework::Extra)
{

void AbilitySystemInit(
    AActor *Owner,
    bool &bInOutAbilitySystemInitialized,
    UAbilitySystemComponent *AbilitySystemComponent,
    TArray<TSubclassOf<UGameplayAbility>> InitialCharacterAbilities,
    TSubclassOf<UGameplayEffect> InitialDefaultAttributes,
    FActiveGameplayEffectHandle &OutDefaultGameplayEffectHandle);

}

namespace Redpoint::EOS::Framework::Extra
{
REDPOINT_EOS_FILE_NS_EXPORT(3297874782, Redpoint::EOS::Framework::Extra, AbilitySystemInit)
}

REDPOINT_EOS_CODE_GUARD_END()