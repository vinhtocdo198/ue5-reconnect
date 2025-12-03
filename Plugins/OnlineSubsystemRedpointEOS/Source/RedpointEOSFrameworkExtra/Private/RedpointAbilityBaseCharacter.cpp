// Copyright June Rhodes. All Rights Reserved.

#include "RedpointAbilityBaseCharacter.h"

#include "AbilitySystemComponent.h"
#include "Components/InputComponent.h"
#include "LogRedpointEOSFrameworkExtra.h"
#include "Net/UnrealNetwork.h"
#include "RedpointAbilityCharacterMovementComponent.h"
#include "RedpointEOSFrameworkExtra/AbilitySystemInit.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

ARedpointAbilityBaseCharacter::ARedpointAbilityBaseCharacter(const class FObjectInitializer &ObjectInitializer)
    : Super(ObjectInitializer.SetDefaultSubobjectClass<URedpointAbilityCharacterMovementComponent>(
          ACharacter::CharacterMovementComponentName))
{
    this->LastClientPlayerState = nullptr;
    this->bASCInputBound = false;

    // Create the volatile ability system component, for characters that are not possessed by a controller.
    this->VolatileAbilitySystem = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("VolatileAbilitySystem"));
    this->VolatileAbilitySystem->SetIsReplicated(true);

    // We haven't initialized the volatile ability system yet.
    this->bVolatileAbilitySystemInitialized = false;

    // Mixed mode means we only are replicated the GEs to ourself, not the GEs to simulated proxies. If another
    // character receives a GE, we won't be told about it by the Server. Attributes, GameplayTags, and GameplayCues will
    // still replicate to us.
    this->VolatileAbilitySystem->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
}

void ARedpointAbilityBaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void ARedpointAbilityBaseCharacter::BeginPlay()
{
    Super::BeginPlay();

    auto PS = this->GetPlayerState<ARedpointAbilityBasePlayerState>();
    if (!PS)
    {
        // Initialize the volatile system.
        FActiveGameplayEffectHandle DefaultGameplayEffectHandle;
        ::Redpoint::EOS::Framework::Extra::AbilitySystemInit(
            this,
            this->bVolatileAbilitySystemInitialized,
            this->VolatileAbilitySystem,
            this->VolatileCharacterAbilities,
            this->VolatileDefaultAttributes,
            DefaultGameplayEffectHandle);
    }
}

UAbilitySystemComponent *ARedpointAbilityBaseCharacter::GetAbilitySystemComponent() const
{
    auto PS = this->GetPlayerState<ARedpointAbilityBasePlayerState>();

    if (IsValid(PS))
    {
        return PS->AbilitySystem;
    }

    checkf(
        this->VolatileAbilitySystem != nullptr,
        TEXT("VolatileAbilitySystem should be initialized before GetAbilitySystemComponent is ever called!"));
    return this->VolatileAbilitySystem;
};

void ARedpointAbilityBaseCharacter::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Bind ASC input, also called in OnRep_PlayerState due to replication race condition.
    this->BindASCInput();
}

void ARedpointAbilityBaseCharacter::PossessedBy(AController *InController)
{
    Super::PossessedBy(InController);

    auto PS = this->GetPlayerState<ARedpointAbilityBasePlayerState>();

    if (IsValid(PS))
    {
        // We're now being possessed by a new controller, so set us as the avatar
        // actor for the ASC.
        PS->AbilitySystem->InitAbilityActorInfo(PS, this);
    }
}

void ARedpointAbilityBaseCharacter::UnPossessed()
{
    auto PS = this->GetPlayerState<ARedpointAbilityBasePlayerState>();

    if (IsValid(PS))
    {
        // We're no longer being possessed by the previous controller, so remove us
        // as the avatar actor for the ASC.
        PS->AbilitySystem->InitAbilityActorInfo(PS, nullptr);
    }

    Super::UnPossessed();

    // After Super::UnPossessed on the server, player state will now be nullptr. Initialize the volatile ability system
    // if needed.
    PS = this->GetPlayerState<ARedpointAbilityBasePlayerState>();
    if (!PS)
    {
        // Initialize the volatile system.
        FActiveGameplayEffectHandle DefaultGameplayEffectHandle;
        ::Redpoint::EOS::Framework::Extra::AbilitySystemInit(
            this,
            this->bVolatileAbilitySystemInitialized,
            this->VolatileAbilitySystem,
            this->VolatileCharacterAbilities,
            this->VolatileDefaultAttributes,
            DefaultGameplayEffectHandle);
    }
}

void ARedpointAbilityBaseCharacter::OnRep_PlayerState()
{
    // We must call Super::OnRep_PlayerState, or the client player states won't have a reference to the controlled
    // pawns.
    Super::OnRep_PlayerState();

    if (this->LastClientPlayerState != nullptr)
    {
        auto LPS = Cast<ARedpointAbilityBasePlayerState>(this->LastClientPlayerState);

        if (IsValid(LPS))
        {
            // On the client, we need to disassociate ourselves from the previous player
            // state so we're not being controlled by two ASCs at once.
            if (LPS->AbilitySystem->GetAvatarActor() == this)
            {
                LPS->AbilitySystem->InitAbilityActorInfo(LPS, nullptr);
            }
        }

        this->LastClientPlayerState = nullptr;
    }

    auto PS = this->GetPlayerState<ARedpointAbilityBasePlayerState>();

    if (IsValid(PS))
    {
        // On the client, we need to associate ourselves with the ASC when PlayerState
        // gets assigned to the character - we don't receive PossessedBy or UnPossessed
        // calls on the client.
        PS->AbilitySystem->InitAbilityActorInfo(PS, this);

        // Bind ASC input, also called in SetupPlayerInputComponent due to replication race condition.
        this->BindASCInput();

        // Track PlayerState in LastClientPlayerState field in case we need to remove
        // ourselves later.
        this->LastClientPlayerState = PS;
    }
}

void ARedpointAbilityBaseCharacter::BindASCInput()
{
    auto AbilitySystem = this->GetAbilitySystemComponent();
    if (!this->bASCInputBound && IsValid(AbilitySystem) && IsValid(this->InputComponent))
    {
        // @todo
        /*
        AbilitySystem->BindAbilityActivationToInputComponent(
            this->InputComponent,
            FGameplayAbilityInputBinds("ConfirmInput", "CancelInput", "AbilityInput"));
        */

        this->bASCInputBound = true;
    }
}

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()