// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "GameplayAbilitySpec.h"
#include "RedpointAbilityBasePlayerState.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointAbilityBaseCharacter.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

UCLASS(BlueprintType, Abstract, Category = "Redpoint EOS Online Framework|Gameplay Ability System")
class REDPOINTEOSFRAMEWORKEXTRA_API ARedpointAbilityBaseCharacter : public ACharacter, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
    ARedpointAbilityBaseCharacter(const FObjectInitializer &ObjectInitializer);

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;

    virtual void BeginPlay() override;

    virtual UAbilitySystemComponent *GetAbilitySystemComponent() const override;

    virtual void SetupPlayerInputComponent(class UInputComponent *PlayerInputComponent) override;

    // Update the ASC in the PlayerState whenever this character is possessed/unpossessed, so that ability
    // tasks running in the ASC will use this character as the AvatarActor.
    virtual void PossessedBy(AController *InController) override; // Server Only
    virtual void UnPossessed() override;                          // Server Only
    virtual void OnRep_PlayerState() override;                    // Client Only

private:
    // On the client we receive OnRep_PlayerState, but this doesn't provide the previous value in it's signature. If our
    // PlayerState is changing on the client, we need to remove ourselves as the avatar actor from any previous
    // PlayerState (otherwise we might be the avatar for two ASCs). This field tracks the previous PlayerState so we can
    // unset ourselves when the PlayerState field updates.
    UPROPERTY()
    TObjectPtr<APlayerState> LastClientPlayerState = nullptr;

    // Called from both SetupPlayerInputComponent and OnRep_PlayerState because of a potential race condition where the
    // PlayerController might call ClientRestart which calls SetupPlayerInputComponent before the PlayerState is repped
    // to the client so the PlayerState would be null in SetupPlayerInputComponent. Conversely, the PlayerState might be
    // repped before the PlayerController calls ClientRestart so the Actor's InputComponent would be null in
    // OnRep_PlayerState.
    void BindASCInput();
    bool bASCInputBound;

    // The ability system that is used when this character is not possessed.
    UPROPERTY()
    TObjectPtr<UAbilitySystemComponent> VolatileAbilitySystem;

    // If true, the volatile ability system has gone through initialization.
    bool bVolatileAbilitySystemInitialized;

public:
    // If this character is not possessed by a player or created for a player controller, these are the character
    // abilities that are automatically granted and added to the volatile ability system component. This setting has no
    // effect for characters possessed by a controller.
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Abilities")
    TArray<TSubclassOf<UGameplayAbility>> VolatileCharacterAbilities;

    // If this character is not possessed by a player or created for a player controller, these are the gameplay effects
    // that initialize default attributes on the volatile ability system component. This setting has no effect for
    // characters possessed by a controller.
    UPROPERTY(
        BlueprintReadOnly,
        EditAnywhere,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Abilities")
    TSubclassOf<UGameplayEffect> VolatileDefaultAttributes;
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()