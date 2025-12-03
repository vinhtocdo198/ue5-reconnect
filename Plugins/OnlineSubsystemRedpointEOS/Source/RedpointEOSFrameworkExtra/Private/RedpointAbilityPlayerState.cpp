// Copyright June Rhodes. All Rights Reserved.

#include "RedpointAbilityPlayerState.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

ARedpointAbilityPlayerState::ARedpointAbilityPlayerState()
    : ARedpointAbilityBasePlayerState()
{
    this->Backpack = CreateDefaultSubobject<URedpointBackpackComponent>(TEXT("RedpointBackpack"));

    this->DashAttributeSet = CreateDefaultSubobject<URedpointAttributeSetDash>(TEXT("RedpointAttributeSetDash"));
    this->HealthAttributeSet = CreateDefaultSubobject<URedpointAttributeSetHealth>(TEXT("RedpointAttributeSetHealth"));
    this->InteractAttributeSet =
        CreateDefaultSubobject<URedpointAttributeSetInteract>(TEXT("RedpointAttributeSetInteract"));
    this->JetpackAttributeSet =
        CreateDefaultSubobject<URedpointAttributeSetJetpack>(TEXT("RedpointAttributeSetJetpack"));
    this->MovementAttributeSet =
        CreateDefaultSubobject<URedpointAttributeSetMovement>(TEXT("RedpointAttributeSetMovement"));
    this->StunAttributeSet = CreateDefaultSubobject<URedpointAttributeSetStun>(TEXT("RedpointAttributeSetStun"));
}

bool ARedpointAbilityPlayerState::IsAlive_Implementation()
{
    return this->HealthAttributeSet->GetCurrentHealth() > 0.0f;
}

URedpointBackpackComponent *ARedpointAbilityPlayerState::GetBackpack_Implementation()
{
    return this->Backpack;
}

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()