// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "UObject/Interface.h"
#include "UObject/ObjectMacros.h"

#include "RedpointOverridesMaxSpeedInterface.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

UINTERFACE(MinimalAPI, Blueprintable, Category = "Redpoint EOS Online Framework|Gameplay Ability System")
class URedpointOverridesMaxSpeedInterface : public UInterface
{
    GENERATED_BODY()
};

class IRedpointOverridesMaxSpeedInterface
{
    GENERATED_BODY()

public:
    UFUNCTION(
        BlueprintCallable,
        BlueprintNativeEvent,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System",
        Meta = (DisplayName = "Get Max Speed"))
    float GetMaxSpeed();
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()