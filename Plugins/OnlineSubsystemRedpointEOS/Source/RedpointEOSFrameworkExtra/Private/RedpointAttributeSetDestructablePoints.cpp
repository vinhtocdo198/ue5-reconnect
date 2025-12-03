// Copyright June Rhodes. All Rights Reserved.

#include "RedpointAttributeSetDestructablePoints.h"

#include "Net/UnrealNetwork.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

void URedpointAttributeSetDestructablePoints::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty> &OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(
        URedpointAttributeSetDestructablePoints,
        TotalPointsForDestruction,
        COND_None,
        REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(
        URedpointAttributeSetDestructablePoints,
        FinalHitPointsForDestruction,
        COND_None,
        REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(
        URedpointAttributeSetDestructablePoints,
        TotalPointsDroppedOnDestruction,
        COND_None,
        REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(
        URedpointAttributeSetDestructablePoints,
        PointsForRecoveryToExitAfterStun,
        COND_None,
        REPNOTIFY_Always);
}

void URedpointAttributeSetDestructablePoints::OnRep_TotalPointsForDestruction(const FGameplayAttributeData &OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(URedpointAttributeSetDestructablePoints, TotalPointsForDestruction, OldValue);
}

void URedpointAttributeSetDestructablePoints::OnRep_FinalHitPointsForDestruction(const FGameplayAttributeData &OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(URedpointAttributeSetDestructablePoints, FinalHitPointsForDestruction, OldValue);
}

void URedpointAttributeSetDestructablePoints::OnRep_TotalPointsDroppedOnDestruction(
    const FGameplayAttributeData &OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(URedpointAttributeSetDestructablePoints, TotalPointsDroppedOnDestruction, OldValue);
}

void URedpointAttributeSetDestructablePoints::OnRep_PointsForRecoveryToExitAfterStun(
    const FGameplayAttributeData &OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(URedpointAttributeSetDestructablePoints, PointsForRecoveryToExitAfterStun, OldValue);
}

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()