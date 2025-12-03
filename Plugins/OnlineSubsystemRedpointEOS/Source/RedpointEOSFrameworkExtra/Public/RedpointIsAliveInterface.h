// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "UObject/Interface.h"
#include "UObject/ObjectMacros.h"

#include "RedpointIsAliveInterface.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

UINTERFACE(MinimalAPI, Blueprintable, Category = "Redpoint EOS Online Framework|Gameplay Ability System")
class URedpointIsAliveInterface : public UInterface
{
    GENERATED_BODY()
};

class REDPOINTEOSFRAMEWORKEXTRA_API IRedpointIsAliveInterface
{
    GENERATED_BODY()

public:
    UFUNCTION(
        BlueprintCallable,
        BlueprintNativeEvent,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System",
        Meta = (DisplayName = "Is Alive?"))
    bool IsAlive();
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()