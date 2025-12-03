// Copyright June Rhodes. All Rights Reserved.

#include "RedpointAttributeSetInteract.h"

#include "Net/UnrealNetwork.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

void URedpointAttributeSetInteract::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(URedpointAttributeSetInteract, InteractDistance, COND_None, REPNOTIFY_Always);
}

void URedpointAttributeSetInteract::OnRep_InteractDistance(const FGameplayAttributeData &OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(URedpointAttributeSetInteract, InteractDistance, OldValue);
}

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()