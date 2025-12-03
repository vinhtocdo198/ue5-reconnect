// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "UObject/Interface.h"
#include "UObject/ObjectMacros.h"

#include "RedpointNotifyUniqueIdInterface.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

/**
 * Components on player states that implemented this interface will be notified when the player state has it's unique
 * net ID initially set, provided that you are using RedpointGameSession as your AGameSession implementation.
 */
UINTERFACE(MinimalAPI, Blueprintable, Category = "Redpoint EOS Online Framework")
class URedpointNotifyUniqueIdInterface : public UInterface
{
    GENERATED_BODY()
};

class IRedpointNotifyUniqueIdInterface
{
    GENERATED_BODY()

public:
    UFUNCTION(
        BlueprintCallable,
        BlueprintNativeEvent,
        Category = "Redpoint EOS Online Framework",
        Meta = (DisplayName = "Player State Set to Unique Net Id"))
    void NotifyUniqueNetIdSet(APlayerState *PlayerState, const FUniqueNetIdRepl &UniqueId);
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()