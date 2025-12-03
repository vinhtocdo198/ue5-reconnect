// Copyright June Rhodes. All Rights Reserved.

#include "RedpointAbilityBasePlayerState.h"

#include "AbilitySystemComponent.h"
#include "GameFramework/Controller.h"
#include "RedpointEOSFrameworkExtra/AbilitySystemInit.h"
#include "UObject/UnrealType.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

ARedpointAbilityBasePlayerState::ARedpointAbilityBasePlayerState()
{
    // Create ability system component, and set it to be explicitly replicated
    this->AbilitySystem = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystem"));
    this->AbilitySystem->SetIsReplicated(true);

    // Mixed mode means we only are replicated the GEs to ourself, not the GEs to simulated proxies. If another
    // character receives a GE, we won't be told about it by the Server. Attributes, GameplayTags, and GameplayCues will
    // still replicate to us.
    this->AbilitySystem->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
}

UAbilitySystemComponent *ARedpointAbilityBasePlayerState::GetAbilitySystemComponent() const
{
    return this->AbilitySystem;
};

void ARedpointAbilityBasePlayerState::BeginPlay()
{
    Super::BeginPlay();

    // Initialize our ability system.
    bool bInitialized = false;
    ::Redpoint::EOS::Framework::Extra::AbilitySystemInit(
        this,
        bInitialized,
        this->AbilitySystem,
        this->CharacterAbilities,
        this->DefaultAttributes,
        this->DefaultGameplayEffectHandle);
}

void ARedpointAbilityBasePlayerState::ResetGameplayAbilities()
{
    // Remove the existing effect.
    this->AbilitySystem->RemoveActiveGameplayEffect(this->DefaultGameplayEffectHandle);

    // Apply the new default effect.
    if (this->DefaultAttributes)
    {
        FGameplayEffectContextHandle EffectContext = this->AbilitySystem->MakeEffectContext();
        EffectContext.AddSourceObject(this);

        FGameplayEffectSpecHandle NewHandle =
            this->AbilitySystem->MakeOutgoingSpec(this->DefaultAttributes, 1, EffectContext);
        if (NewHandle.IsValid())
        {
            this->DefaultGameplayEffectHandle =
                this->AbilitySystem->ApplyGameplayEffectSpecToSelf(*NewHandle.Data.Get());
        }
    }
}

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()