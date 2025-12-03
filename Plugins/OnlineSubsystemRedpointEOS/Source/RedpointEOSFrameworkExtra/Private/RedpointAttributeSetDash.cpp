// Copyright June Rhodes. All Rights Reserved.

#include "RedpointAttributeSetDash.h"

#include "Net/UnrealNetwork.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

void URedpointAttributeSetDash::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(URedpointAttributeSetDash, DashLengthTime, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(URedpointAttributeSetDash, DashStrength, COND_None, REPNOTIFY_Always);
}

void URedpointAttributeSetDash::OnRep_DashLengthTime(const FGameplayAttributeData &OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(URedpointAttributeSetDash, DashLengthTime, OldValue);
}

void URedpointAttributeSetDash::OnRep_DashStrength(const FGameplayAttributeData &OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(URedpointAttributeSetDash, DashStrength, OldValue);
}

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()