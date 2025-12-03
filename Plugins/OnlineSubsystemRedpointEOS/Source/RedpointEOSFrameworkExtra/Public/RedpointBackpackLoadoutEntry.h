// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointBackpackItem.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointBackpackLoadoutEntry.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

USTRUCT()
struct REDPOINTEOSFRAMEWORKEXTRA_API FRedpointBackpackLoadoutEntry
{
    GENERATED_BODY()

    /** The item to grant when the Backpack is created. */
    UPROPERTY(EditDefaultsOnly, Category = "Redpoint EOS Online Framework|Gameplay Ability System|Backpack")
    TSubclassOf<ARedpointBackpackItem> Item;
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()