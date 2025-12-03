// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointAttributeSetBase.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointAttributeSetReserveAmmo.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

/**
 * This provides attributes for a single type of ammo reserve that the player can carry. This is not suitable for most
 * games, as it does not differentiate between different types of ammo that a player could carry.
 */
UCLASS(BlueprintType, Category = "Redpoint EOS Online Framework|Gameplay Ability System|Attribute Sets")
class REDPOINTEOSFRAMEWORKEXTRA_API URedpointAttributeSetReserveAmmo : public URedpointAttributeSetBase
{
    GENERATED_BODY()

public:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;

    /**
     * The reserve ammo the player is currently carrying.
     */
    UPROPERTY(
        BlueprintReadOnly,
        ReplicatedUsing = OnRep_CurrentReserveAmmo,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Attributes",
        Meta = (DisplayName = "Current Reserve Ammo"))
    FGameplayAttributeData CurrentReserveAmmo;
    REDPOINT_FRAMEWORK_ATTRIBUTE_ACCESSORS(URedpointAttributeSetReserveAmmo, CurrentReserveAmmo)

    /**
     * The maximum amount of reserve ammo that the player can carry.
     */
    UPROPERTY(
        BlueprintReadOnly,
        ReplicatedUsing = OnRep_MaximumReserveAmmo,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Attributes",
        Meta = (DisplayName = "Maximum Reserve Ammo"))
    FGameplayAttributeData MaximumReserveAmmo;
    REDPOINT_FRAMEWORK_ATTRIBUTE_ACCESSORS(URedpointAttributeSetReserveAmmo, MaximumReserveAmmo)

    virtual void PreAttributeChange(const FGameplayAttribute &Attribute, float &NewValue) override;
    virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData &Data) override;

protected:
    UFUNCTION()
    virtual void OnRep_CurrentReserveAmmo(const FGameplayAttributeData &OldValue);

    UFUNCTION()
    virtual void OnRep_MaximumReserveAmmo(const FGameplayAttributeData &OldValue);
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()