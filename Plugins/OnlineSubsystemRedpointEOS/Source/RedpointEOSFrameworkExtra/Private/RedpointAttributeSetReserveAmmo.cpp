// Copyright June Rhodes. All Rights Reserved.

#include "RedpointAttributeSetReserveAmmo.h"

#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

void URedpointAttributeSetReserveAmmo::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(URedpointAttributeSetReserveAmmo, CurrentReserveAmmo, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(URedpointAttributeSetReserveAmmo, MaximumReserveAmmo, COND_None, REPNOTIFY_Always);
}

void URedpointAttributeSetReserveAmmo::PreAttributeChange(const FGameplayAttribute &Attribute, float &NewValue)
{
    Super::PreAttributeChange(Attribute, NewValue);
}

void URedpointAttributeSetReserveAmmo::PostGameplayEffectExecute(const FGameplayEffectModCallbackData &Data)
{
    Super::PostGameplayEffectExecute(Data);

    if (Data.EvaluatedData.Attribute == GetCurrentReserveAmmoAttribute())
    {
        float Ammo = GetCurrentReserveAmmo();
        SetCurrentReserveAmmo(FMath::Clamp<float>(Ammo, 0, GetMaximumReserveAmmo()));
    }
}

void URedpointAttributeSetReserveAmmo::OnRep_CurrentReserveAmmo(const FGameplayAttributeData &OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(URedpointAttributeSetReserveAmmo, CurrentReserveAmmo, OldValue);
}

void URedpointAttributeSetReserveAmmo::OnRep_MaximumReserveAmmo(const FGameplayAttributeData &OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(URedpointAttributeSetReserveAmmo, MaximumReserveAmmo, OldValue);
}

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()