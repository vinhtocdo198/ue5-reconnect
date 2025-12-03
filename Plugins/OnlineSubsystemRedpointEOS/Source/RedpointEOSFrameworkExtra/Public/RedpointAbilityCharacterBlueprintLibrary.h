// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Abilities/GameplayAbility.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "RedpointAbilityBaseCharacter.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointAbilityCharacterBlueprintLibrary.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

UCLASS()
class REDPOINTEOSFRAMEWORKEXTRA_API URedpointAbilityCharacterBlueprintLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(
        BlueprintCallable,
        BlueprintAuthorityOnly,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Characters")
    void GiveAbilityFromClass(
        ARedpointAbilityBaseCharacter *Character,
        TSubclassOf<class UGameplayAbility> AbilityClass,
        int32 Level,
        int32 InputIndex,
        FGameplayAbilitySpecHandle &Handle,
        bool &Success);

    UFUNCTION(
        BlueprintCallable,
        BlueprintAuthorityOnly,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Characters")
    void ClearAbilityFromClass(
        ARedpointAbilityBaseCharacter *Character,
        const FGameplayAbilitySpecHandle &Handle,
        bool &Success);

    UFUNCTION(
        BlueprintCallable,
        BlueprintAuthorityOnly,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Characters")
    void ClearAllAbilities(ARedpointAbilityBaseCharacter *Character);

    UFUNCTION(BlueprintCallable, Category = "Redpoint EOS Online Framework|Gameplay Ability System|Abilities")
    static UGameplayAbility *GetPrimaryAbilityInstanceFromHandle(
        UAbilitySystemComponent *AbilitySystemComponent,
        FGameplayAbilitySpecHandle Handle);

    UFUNCTION(BlueprintCallable, Category = "Redpoint EOS Online Framework|Gameplay Ability System|Abilities")
    static UGameplayAbility *GetPrimaryAbilityInstanceFromClass(
        UAbilitySystemComponent *AbilitySystemComponent,
        TSubclassOf<UGameplayAbility> InAbilityClass);

    UFUNCTION(BlueprintCallable, Category = "Redpoint EOS Online Framework|Gameplay Ability System|Abilities")
    static bool IsPrimaryAbilityInstanceActive(
        UAbilitySystemComponent *AbilitySystemComponent,
        FGameplayAbilitySpecHandle Handle);

    UFUNCTION(
        BlueprintCallable,
        BlueprintPure,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Abilities")
    static bool IsAbilitySpecHandleValid(FGameplayAbilitySpecHandle Handle);

    UFUNCTION(
        BlueprintCallable,
        BlueprintPure,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Abilities")
    static FGameplayAbilitySpecHandle FindAbilitySpecHandleForClass(
        UAbilitySystemComponent *AbilitySystem,
        TSubclassOf<UGameplayAbility> AbilityClass,
        UObject *OptionalSourceObject = nullptr);
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()