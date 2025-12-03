// Copyright June Rhodes. All Rights Reserved.

#include "RedpointAbilityPlayerCharacter.h"

#include "RedpointAbilityPlayerState.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

bool ARedpointAbilityPlayerCharacter::IsAlive_Implementation()
{
    auto PS = this->GetPlayerState<ARedpointAbilityPlayerState>();

    if (IsValid(PS))
    {
        return PS->IsAlive_Implementation();
    }

    return false;
}

URedpointBackpackComponent *ARedpointAbilityPlayerCharacter::GetBackpack_Implementation()
{
    auto PS = this->GetPlayerState<ARedpointAbilityPlayerState>();

    if (IsValid(PS))
    {
        return PS->GetBackpack_Implementation();
    }

    return nullptr;
}

float ARedpointAbilityPlayerCharacter::GetMaxSpeed_Implementation()
{
    auto PS = this->GetPlayerState<ARedpointAbilityPlayerState>();

    if (IsValid(PS))
    {
        return PS->MovementAttributeSet->MovementSpeed.GetCurrentValue();
    }

    return 0.f;
}

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()