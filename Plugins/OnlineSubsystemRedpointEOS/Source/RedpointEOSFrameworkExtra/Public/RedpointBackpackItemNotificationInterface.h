// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointBackpackItem.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "UObject/Interface.h"
#include "UObject/ObjectMacros.h"

#include "RedpointBackpackItemNotificationInterface.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

UINTERFACE(MinimalAPI, Blueprintable, Category = "Redpoint EOS Online Framework|Gameplay Ability System|Backpack")
class URedpointBackpackItemNotificationInterface : public UInterface
{
    GENERATED_BODY()
};

class REDPOINTEOSFRAMEWORKEXTRA_API IRedpointBackpackItemNotificationInterface
{
    GENERATED_BODY()

public:
    UFUNCTION(
        BlueprintCallable,
        BlueprintNativeEvent,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Backpack",
        Meta = (DisplayName = "On Item Changed"))
    void OnItemChanged(ARedpointBackpackItem *NewItem, ARedpointBackpackItem *PreviousItem);
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()