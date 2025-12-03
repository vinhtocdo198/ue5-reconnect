// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointAbilityBaseCharacter.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointHasBackpackInterface.h"
#include "RedpointIsAliveInterface.h"
#include "RedpointOverridesMaxSpeedInterface.h"

#include "RedpointAbilityPlayerCharacter.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

UCLASS(BlueprintType, Category = "Redpoint EOS Online Framework|Gameplay Ability System")
class REDPOINTEOSFRAMEWORKEXTRA_API ARedpointAbilityPlayerCharacter : public ARedpointAbilityBaseCharacter,
                                                                 public IRedpointIsAliveInterface,
                                                                 public IRedpointHasBackpackInterface,
                                                                 public IRedpointOverridesMaxSpeedInterface
{
    GENERATED_BODY()

public:
    bool IsAlive_Implementation() override;

    URedpointBackpackComponent *GetBackpack_Implementation() override;

    float GetMaxSpeed_Implementation() override;
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()