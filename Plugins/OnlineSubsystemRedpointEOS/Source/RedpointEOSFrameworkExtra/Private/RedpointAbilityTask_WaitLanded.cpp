// Copyright June Rhodes. All Rights Reserved.

#include "RedpointAbilityTask_WaitLanded.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

URedpointAbilityTask_WaitLanded::URedpointAbilityTask_WaitLanded(const FObjectInitializer &ObjectInitializer)
    : Super(ObjectInitializer)
{
}

URedpointAbilityTask_WaitLanded *URedpointAbilityTask_WaitLanded::CreateWaitLanded(
    class UGameplayAbility *OwningAbility)
{
    URedpointAbilityTask_WaitLanded *MyObj = NewAbilityTask<URedpointAbilityTask_WaitLanded>(OwningAbility);
    return MyObj;
}

void URedpointAbilityTask_WaitLanded::Activate()
{
    ACharacter *Character = Cast<ACharacter>(GetAvatarActor());
    if (Character)
    {
        Character->LandedDelegate.AddDynamic(this, &URedpointAbilityTask_WaitLanded::HandleOnLanded);
        MyCharacter = Character;
    }

    SetWaitingOnAvatar();
}

void URedpointAbilityTask_WaitLanded::HandleOnLanded(const FHitResult &Hit)
{
    if (ShouldBroadcastAbilityTaskDelegates())
    {
        OnLanded.Broadcast(Hit);
    }
    EndTask();
}

void URedpointAbilityTask_WaitLanded::OnDestroy(bool AbilityEnded)
{
    if (MyCharacter.IsValid())
    {
        MyCharacter->LandedDelegate.RemoveDynamic(this, &URedpointAbilityTask_WaitLanded::HandleOnLanded);
    }

    Super::OnDestroy(AbilityEnded);
}

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()