// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointAttributeSetBase.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointAttributeSetHealth.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

UCLASS(BlueprintType, Category = "Redpoint EOS Online Framework|Gameplay Ability System|Attribute Sets")
class REDPOINTEOSFRAMEWORKEXTRA_API URedpointAttributeSetHealth : public URedpointAttributeSetBase
{
    GENERATED_BODY()

public:
    virtual void PreAttributeChange(const FGameplayAttribute &Attribute, float &NewValue) override;
    virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData &Data) override;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;

    /**
     * The current health.
     */
    UPROPERTY(
        BlueprintReadOnly,
        ReplicatedUsing = OnRep_CurrentHealth,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Attributes",
        Meta = (DisplayName = "Current Health"))
    FGameplayAttributeData CurrentHealth;
    REDPOINT_FRAMEWORK_ATTRIBUTE_ACCESSORS(URedpointAttributeSetHealth, CurrentHealth)

    /**
     * The maximum health.
     */
    UPROPERTY(
        BlueprintReadOnly,
        ReplicatedUsing = OnRep_MaximumHealth,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Attributes",
        Meta = (DisplayName = "Maximum Health"))
    FGameplayAttributeData MaximumHealth;
    REDPOINT_FRAMEWORK_ATTRIBUTE_ACCESSORS(URedpointAttributeSetHealth, MaximumHealth)

    /**
     * The amount of time in seconds since being last hit until health starts to regenerate (0 = indefinite, no health
     * regen).
     */
    UPROPERTY(
        BlueprintReadOnly,
        ReplicatedUsing = OnRep_HealthRegenDelay,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Attributes",
        Meta = (DisplayName = "Health Regen Delay (seconds)"))
    FGameplayAttributeData HealthRegenDelay;
    REDPOINT_FRAMEWORK_ATTRIBUTE_ACCESSORS(URedpointAttributeSetHealth, HealthRegenDelay)

    /**
     * The amount of health to regenerate per tick.
     */
    UPROPERTY(
        BlueprintReadOnly,
        ReplicatedUsing = OnRep_HealthRegenAmount,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Attributes",
        Meta = (DisplayName = "Health Regen Amount (per tick)"))
    FGameplayAttributeData HealthRegenAmount;
    REDPOINT_FRAMEWORK_ATTRIBUTE_ACCESSORS(URedpointAttributeSetHealth, HealthRegenAmount)

    /**
     * Percentage value of your remaining health that modifies the base regen amount (0 = health regen amount should be
     * applied constantly, no modifier).
     */
    UPROPERTY(
        BlueprintReadOnly,
        ReplicatedUsing = OnRep_HealthRegenAmountModifier,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Attributes",
        Meta = (DisplayName = "Health Regen Amount Modifier (%)"))
    FGameplayAttributeData HealthRegenAmountModifier;
    REDPOINT_FRAMEWORK_ATTRIBUTE_ACCESSORS(URedpointAttributeSetHealth, HealthRegenAmountModifier)

    /**
     * Damage is a meta attribute used by the DamageExecution to calculate final damage, which then turns into -Health
     * Temporary value that only exists on the Server. Not replicated.
     */
    UPROPERTY(
        BlueprintReadOnly,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Attributes",
        Meta = (HideFromLevelInfos, DisplayName = "Damage"))
    FGameplayAttributeData Damage;
    REDPOINT_FRAMEWORK_ATTRIBUTE_ACCESSORS(URedpointAttributeSetHealth, Damage)

protected:
    UFUNCTION()
    virtual void OnRep_CurrentHealth(const FGameplayAttributeData &OldValue);

    UFUNCTION()
    virtual void OnRep_MaximumHealth(const FGameplayAttributeData &OldValue);

    UFUNCTION()
    virtual void OnRep_HealthRegenDelay(const FGameplayAttributeData &OldValue);

    UFUNCTION()
    virtual void OnRep_HealthRegenAmount(const FGameplayAttributeData &OldValue);

    UFUNCTION()
    virtual void OnRep_HealthRegenAmountModifier(const FGameplayAttributeData &OldValue);
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()