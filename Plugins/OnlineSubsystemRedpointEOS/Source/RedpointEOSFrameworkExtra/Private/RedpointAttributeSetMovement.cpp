// Copyright June Rhodes. All Rights Reserved.

#include "RedpointAttributeSetMovement.h"

#include "Net/UnrealNetwork.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

void URedpointAttributeSetMovement::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(URedpointAttributeSetMovement, MovementSpeed, COND_None, REPNOTIFY_Always);
}

void URedpointAttributeSetMovement::OnRep_MovementSpeed(const FGameplayAttributeData &OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(URedpointAttributeSetMovement, MovementSpeed, OldValue);
}

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()