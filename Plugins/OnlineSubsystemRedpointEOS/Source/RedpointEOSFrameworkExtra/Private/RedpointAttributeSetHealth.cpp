// Copyright June Rhodes. All Rights Reserved.

#include "RedpointAttributeSetHealth.h"

#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameplayEffectExtension.h"
#include "LogRedpointEOSFrameworkExtra.h"
#include "Net/UnrealNetwork.h"
#include "RedpointAbilityBaseCharacter.h"
#include "RedpointDamageNotificationInterface.h"
#include "RedpointIsAliveInterface.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

void URedpointAttributeSetHealth::PreAttributeChange(const FGameplayAttribute &Attribute, float &NewValue)
{
    Super::PreAttributeChange(Attribute, NewValue);

    if (Attribute == GetMaximumHealthAttribute())
    {
        AdjustAttributeForMaxChange(CurrentHealth, MaximumHealth, NewValue, GetCurrentHealthAttribute());
    }
}

void URedpointAttributeSetHealth::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(URedpointAttributeSetHealth, CurrentHealth, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(URedpointAttributeSetHealth, MaximumHealth, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(URedpointAttributeSetHealth, HealthRegenDelay, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(URedpointAttributeSetHealth, HealthRegenAmount, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(URedpointAttributeSetHealth, HealthRegenAmountModifier, COND_None, REPNOTIFY_Always);
}

void URedpointAttributeSetHealth::OnRep_CurrentHealth(const FGameplayAttributeData &OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(URedpointAttributeSetHealth, CurrentHealth, OldValue);
}

void URedpointAttributeSetHealth::OnRep_MaximumHealth(const FGameplayAttributeData &OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(URedpointAttributeSetHealth, MaximumHealth, OldValue);
}

void URedpointAttributeSetHealth::OnRep_HealthRegenDelay(const FGameplayAttributeData &OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(URedpointAttributeSetHealth, HealthRegenDelay, OldValue);
}

void URedpointAttributeSetHealth::OnRep_HealthRegenAmount(const FGameplayAttributeData &OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(URedpointAttributeSetHealth, HealthRegenAmount, OldValue);
}

void URedpointAttributeSetHealth::OnRep_HealthRegenAmountModifier(const FGameplayAttributeData &OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(URedpointAttributeSetHealth, HealthRegenAmountModifier, OldValue);
}

void URedpointAttributeSetHealth::PostGameplayEffectExecute(const FGameplayEffectModCallbackData &Data)
{
    Super::PostGameplayEffectExecute(Data);

    FGameplayEffectContextHandle Context = Data.EffectSpec.GetContext();
    UAbilitySystemComponent *Source = Context.GetOriginalInstigatorAbilitySystemComponent();
    FGameplayTagContainer SpecAssetTags;
    Data.EffectSpec.GetAllAssetTags(SpecAssetTags);

    // Get the Target actor, which should be our owner
    AActor *TargetActor = nullptr;
    AController *TargetController = nullptr;
    if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
    {
        TargetActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
        TargetController = Data.Target.AbilityActorInfo->PlayerController.Get();
    }

    // Get the Source actor
    AActor *SourceActor = nullptr;
    AController *SourceController = nullptr;
    if (Source && Source->AbilityActorInfo.IsValid() && Source->AbilityActorInfo->AvatarActor.IsValid())
    {
        SourceActor = Source->AbilityActorInfo->AvatarActor.Get();
        SourceController = Source->AbilityActorInfo->PlayerController.Get();
        if (SourceController == nullptr && SourceActor != nullptr)
        {
            if (APawn *Pawn = Cast<APawn>(SourceActor))
            {
                SourceController = Pawn->GetController();
            }
        }

        // Use the controller to find the source pawn
        if (SourceController)
        {
            SourceActor = SourceController->GetPawn();
        }

        // Set the causer actor based on context if it's set
        if (Context.GetEffectCauser())
        {
            SourceActor = Context.GetEffectCauser();
        }
    }

    if (Data.EvaluatedData.Attribute == GetDamageAttribute())
    {
        // Try to extract a hit result
        FHitResult HitResult;
        if (Context.GetHitResult())
        {
            HitResult = *Context.GetHitResult();
        }

        // Store a local copy of the amount of damage done and clear the damage attribute
        const float LocalDamageDone = GetDamage();
        SetDamage(0.f);

        if (LocalDamageDone > 0.0f)
        {
            // If character was alive before damage is added, handle damage
            // This prevents damage being added to dead things and replaying death animations
            bool WasAlive = true;

            if (TargetActor->Implements<URedpointIsAliveInterface>())
            {
                WasAlive = IRedpointIsAliveInterface::Execute_IsAlive(TargetActor);

                if (!WasAlive)
                {
                    UE_LOG(
                        LogRedpointEOSFrameworkExtra,
                        Warning,
                        TEXT("%s() %s is NOT alive when receiving damage"),
                        TEXT("__FUNCTION__"),
                        *TargetActor->GetName());
                }
            }

            // Apply the health change and then clamp it
            const float NewHealth = this->GetCurrentHealth() - LocalDamageDone;
            this->SetCurrentHealth(FMath::Clamp(NewHealth, 0.0f, this->GetMaximumHealth()));

            if (TargetActor && WasAlive)
            {
                // This is the log statement for damage received. Turned off for live games.
                UE_LOG(
                    LogRedpointEOSFrameworkExtra,
                    Verbose,
                    TEXT("%s() %s Damage Received: %f"),
                    TEXT("__FUNCTION__"),
                    *GetOwningActor()->GetName(),
                    static_cast<double>(LocalDamageDone));

                // TODO: We probably want to propagate hit direction to the actor in future, for when we want to drive
                // damage effects and other things in the game. But since it adds new types (enums on hit
                // direction) and we don't yet know what we want here, I've just commented this out until we have a more
                // solid plan.

                /*

                // Play HitReact animation and sound with a multicast RPC.
                const FHitResult *Hit = Data.EffectSpec.GetContext().GetHitResult();

                if (Hit)
                {
                    EGDHitReactDirection HitDirection =
                        TargetCharacter->GetHitReactDirection(Data.EffectSpec.GetContext().GetHitResult()->Location);
                    switch (HitDirection)
                    {
                    case EGDHitReactDirection::Left:
                        TargetCharacter->PlayHitReact(HitDirectionLeftTag, SourceCharacter);
                        break;
                    case EGDHitReactDirection::Front:
                        TargetCharacter->PlayHitReact(HitDirectionFrontTag, SourceCharacter);
                        break;
                    case EGDHitReactDirection::Right:
                        TargetCharacter->PlayHitReact(HitDirectionRightTag, SourceCharacter);
                        break;
                    case EGDHitReactDirection::Back:
                        TargetCharacter->PlayHitReact(HitDirectionBackTag, SourceCharacter);
                        break;
                    }
                }
                else
                {
                    // No hit result. Default to front.
                    TargetCharacter->PlayHitReact(HitDirectionFrontTag, SourceCharacter);
                }

                */

                // Show damage number for the Source player unless it was self damage
                if (SourceActor != TargetActor)
                {
                    if (SourceActor->Implements<URedpointDamageNotificationInterface>())
                    {
                        IRedpointDamageNotificationInterface::Execute_OnDealtDamageToTarget(
                            SourceActor,
                            static_cast<int32>(LocalDamageDone),
                            TargetActor);
                    }
                }

                if (TargetActor->Implements<URedpointIsAliveInterface>() &&
                    !IRedpointIsAliveInterface::Execute_IsAlive(TargetActor))
                {
                    // TargetCharacter was alive before this damage and now is not alive, give XP and Gold bounties to
                    // Source. Don't give bounty to self.
                    if (SourceController != TargetController)
                    {
                        if (SourceActor->Implements<URedpointDamageNotificationInterface>())
                        {
                            IRedpointDamageNotificationInterface::Execute_OnKilledTargetActor(
                                SourceActor,
                                Source,
                                TargetActor,
                                GetOwningAbilitySystemComponent());
                        }

                        // TODO: Example for code that can be put into OnKilledTargetActor in order to generate
                        // dynamic instant gameplay effects.

                        /*

                        // Create a dynamic instant Gameplay Effect to give the bounties
                        UGameplayEffect *GEBounty =
                            NewObject<UGameplayEffect>(GetTransientPackage(), FName(TEXT("Bounty")));
                        GEBounty->DurationPolicy = EGameplayEffectDurationType::Instant;

                        int32 Idx = GEBounty->Modifiers.Num();
                        GEBounty->Modifiers.SetNum(Idx + 2);

                        FGameplayModifierInfo &InfoXP = GEBounty->Modifiers[Idx];
                        InfoXP.ModifierMagnitude = FScalableFloat(GetXPBounty());
                        InfoXP.ModifierOp = EGameplayModOp::Additive;
                        InfoXP.Attribute = UGDAttributeSetBase::GetXPAttribute();

                        FGameplayModifierInfo &InfoGold = GEBounty->Modifiers[Idx + 1];
                        InfoGold.ModifierMagnitude = FScalableFloat(GetGoldBounty());
                        InfoGold.ModifierOp = EGameplayModOp::Additive;
                        InfoGold.Attribute = UGDAttributeSetBase::GetGoldAttribute();

                        Source->ApplyGameplayEffectToSelf(GEBounty, 1.0f, Source->MakeEffectContext());

                        */
                    }
                }
            }
        }

        return;
    }

    if (Data.EvaluatedData.Attribute == this->GetCurrentHealthAttribute())
    {
        // Handle other health changes.
        // Health loss should go through Damage.

        this->SetCurrentHealth(FMath::Clamp(this->GetCurrentHealth(), 0.0f, this->GetMaximumHealth()));
        return;
    }
}

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()