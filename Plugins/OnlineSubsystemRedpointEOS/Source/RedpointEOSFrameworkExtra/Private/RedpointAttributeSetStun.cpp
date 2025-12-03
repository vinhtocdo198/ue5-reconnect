// Copyright June Rhodes. All Rights Reserved.

#include "RedpointAttributeSetStun.h"

#include "Net/UnrealNetwork.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

void URedpointAttributeSetStun::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(URedpointAttributeSetStun, StunTime, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(URedpointAttributeSetStun, StunRecoveryHealthAmount, COND_None, REPNOTIFY_Always);
}

void URedpointAttributeSetStun::OnRep_StunTime(const FGameplayAttributeData &OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(URedpointAttributeSetStun, StunTime, OldValue);
}

void URedpointAttributeSetStun::OnRep_StunRecoveryHealthAmount(const FGameplayAttributeData &OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(URedpointAttributeSetStun, StunRecoveryHealthAmount, OldValue);
}

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()