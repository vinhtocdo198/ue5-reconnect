// Copyright June Rhodes. All Rights Reserved.

#include "RedpointAttributeSetJetpack.h"

#include "Net/UnrealNetwork.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

void URedpointAttributeSetJetpack::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(URedpointAttributeSetJetpack, JetpackLengthTime, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(URedpointAttributeSetJetpack, JetpackStrength, COND_None, REPNOTIFY_Always);
}

void URedpointAttributeSetJetpack::OnRep_JetpackLengthTime(const FGameplayAttributeData &OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(URedpointAttributeSetJetpack, JetpackLengthTime, OldValue);
}

void URedpointAttributeSetJetpack::OnRep_JetpackStrength(const FGameplayAttributeData &OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(URedpointAttributeSetJetpack, JetpackStrength, OldValue);
}

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()