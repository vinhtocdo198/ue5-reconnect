// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointAbilityBasePlayerState.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

UCLASS(BlueprintType, Abstract, Category = "Redpoint EOS Online Framework|Gameplay Ability System")
class REDPOINTEOSFRAMEWORKEXTRA_API ARedpointAbilityBasePlayerState : public APlayerState, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
    ARedpointAbilityBasePlayerState();

    virtual UAbilitySystemComponent *GetAbilitySystemComponent() const override;

    virtual void BeginPlay() override;

    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Abilities",
        meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UAbilitySystemComponent> AbilitySystem;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Abilities")
    TArray<TSubclassOf<UGameplayAbility>> CharacterAbilities;

    // Default attributes for a character for initializing on spawn/respawn.
    // This is an instant GE that overrides the values for attributes that get reset on spawn/respawn.
    UPROPERTY(
        BlueprintReadOnly,
        EditAnywhere,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Abilities")
    TSubclassOf<UGameplayEffect> DefaultAttributes;

private:
    /**
     * Tracks the default gameplay effect ability so we can remove and re-apply it on death.
     */
    FActiveGameplayEffectHandle DefaultGameplayEffectHandle;

    /**
     * Removes and re-applies the default attributes effect to the ability system. This should be called when the
     * character the player is possessing "dies" and you want to reset non-persistent effects.
     */
    UFUNCTION(
        BlueprintCallable,
        BlueprintAuthorityOnly,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Abilities")
    void ResetGameplayAbilities();
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()