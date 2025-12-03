// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Abilities/Tasks/AbilityTask.h"
#include "GameFramework/Character.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "UObject/ObjectMacros.h"

#include "RedpointAbilityTask_WaitLanded.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

UCLASS()
class REDPOINTEOSFRAMEWORKEXTRA_API URedpointAbilityTask_WaitLanded : public UAbilityTask
{
    GENERATED_UCLASS_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FLandedSignature OnLanded;

    UFUNCTION()
    void HandleOnLanded(const FHitResult &Hit);

    /** Wait until the character has landed on a surface */
    UFUNCTION(
        BlueprintCallable,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Tasks",
        meta =
            (HidePin = "OwningAbility",
             DefaultToSelf = "OwningAbility",
             BlueprintInternalUseOnly = "TRUE",
             DisplayName = "Wait Until Character Landed"))
    static URedpointAbilityTask_WaitLanded *CreateWaitLanded(UGameplayAbility *OwningAbility);

    virtual void Activate() override;

private:
    virtual void OnDestroy(bool AbilityEnded) override;

    TWeakObjectPtr<ACharacter> MyCharacter;
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()