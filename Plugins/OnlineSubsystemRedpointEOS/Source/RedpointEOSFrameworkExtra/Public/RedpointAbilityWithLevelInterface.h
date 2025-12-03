// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointBackpackItem.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "UObject/Interface.h"
#include "UObject/ObjectMacros.h"

#include "RedpointAbilityWithLevelInterface.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

UINTERFACE(MinimalAPI, Blueprintable, Category = "Redpoint EOS Online Framework|Gameplay Ability System")
class URedpointAbilityWithLevelInterface : public UInterface
{
    GENERATED_BODY()
};

class REDPOINTEOSFRAMEWORKEXTRA_API IRedpointAbilityWithLevelInterface
{
    GENERATED_BODY()

public:
    UFUNCTION(
        BlueprintCallable,
        BlueprintNativeEvent,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Backpack",
        Meta = (DisplayName = "Get Ability Level for Backpack Item"))
    int32 GetAbilityLevel(ARedpointBackpackItem *Item);
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()