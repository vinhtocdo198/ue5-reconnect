// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointBackpackComponent.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "UObject/Interface.h"
#include "UObject/ObjectMacros.h"

#include "RedpointHasBackpackInterface.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

UINTERFACE(MinimalAPI, Blueprintable, Category = "Redpoint EOS Online Framework|Gameplay Ability System|Backpack")
class URedpointHasBackpackInterface : public UInterface
{
    GENERATED_BODY()
};

class IRedpointHasBackpackInterface
{
    GENERATED_BODY()

public:
    UFUNCTION(
        BlueprintCallable,
        BlueprintNativeEvent,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Backpack",
        Meta = (DisplayName = "Get Backpack"))
    URedpointBackpackComponent *GetBackpack();
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()