// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "AbilitySystemComponent.h"
#include "GameFramework/Actor.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "UObject/Interface.h"
#include "UObject/ObjectMacros.h"

#include "RedpointDamageNotificationInterface.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

UINTERFACE(MinimalAPI, Blueprintable, Category = "Redpoint EOS Online Framework|Gameplay Ability System")
class URedpointDamageNotificationInterface : public UInterface
{
    GENERATED_BODY()
};

class REDPOINTEOSFRAMEWORKEXTRA_API IRedpointDamageNotificationInterface
{
    GENERATED_BODY()

public:
    UFUNCTION(
        BlueprintCallable,
        BlueprintNativeEvent,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System",
        Meta = (DisplayName = "On Dealt Damage to Target"))
    void OnDealtDamageToTarget(int DamageDealt, AActor *TargetActor);

    UFUNCTION(
        BlueprintCallable,
        BlueprintNativeEvent,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System",
        Meta = (DisplayName = "On Killed Target Actor"))
    void OnKilledTargetActor(
        UAbilitySystemComponent *SourceAbilitySystem,
        AActor *TargetActor,
        UAbilitySystemComponent *TargetAbilitySystem);
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()