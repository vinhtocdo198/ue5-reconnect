// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointAbilityBasePlayerState.h"
#include "RedpointAttributeSetDash.h"
#include "RedpointAttributeSetHealth.h"
#include "RedpointAttributeSetInteract.h"
#include "RedpointAttributeSetJetpack.h"
#include "RedpointAttributeSetMovement.h"
#include "RedpointAttributeSetStun.h"
#include "RedpointBackpackComponent.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointHasBackpackInterface.h"
#include "RedpointIsAliveInterface.h"

#include "RedpointAbilityPlayerState.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

UCLASS(BlueprintType, Category = "Redpoint EOS Online Framework|Gameplay Ability System")
class REDPOINTEOSFRAMEWORKEXTRA_API ARedpointAbilityPlayerState : public ARedpointAbilityBasePlayerState,
                                                             public IRedpointIsAliveInterface,
                                                             public IRedpointHasBackpackInterface
{
    GENERATED_BODY()

public:
    ARedpointAbilityPlayerState();

    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Backpack")
    TObjectPtr<URedpointBackpackComponent> Backpack;

    UPROPERTY()
    TObjectPtr<URedpointAttributeSetDash> DashAttributeSet;

    UPROPERTY()
    TObjectPtr<URedpointAttributeSetHealth> HealthAttributeSet;

    UPROPERTY()
    TObjectPtr<URedpointAttributeSetInteract> InteractAttributeSet;

    UPROPERTY()
    TObjectPtr<URedpointAttributeSetJetpack> JetpackAttributeSet;

    UPROPERTY()
    TObjectPtr<URedpointAttributeSetMovement> MovementAttributeSet;

    UPROPERTY()
    TObjectPtr<URedpointAttributeSetStun> StunAttributeSet;

    bool IsAlive_Implementation() override;

    URedpointBackpackComponent *GetBackpack_Implementation() override;
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()