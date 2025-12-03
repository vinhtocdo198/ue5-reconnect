// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Components/ListView.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointFrameworkBlueprintLibrary.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

UCLASS()
class REDPOINTEOSFRAMEWORK_API URedpointFrameworkBlueprintLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /**
     * Set the horizontal or vertical spacing between items on an arbitrary UMG list view, without having to rely on the
     * CommonUI plugin. This function must be called during "Pre Construct", before any items are added to the list
     * view.
     */
    UFUNCTION(
        BlueprintCallable,
        Category = "Redpoint EOS Online Framework|UMG Utilities",
        meta = (DisplayName = "Set Entry Spacing for List View"))
    static void SetListViewEntrySpacing(UListView *InListView, float InEntrySpacing);

    /**
     * Returns true if Redpoint EOS Online Framework is the Free Edition, and thus does not support some runtime
     * features. This blueprint node is used in the example project.
     */
    UFUNCTION(
        BlueprintPure,
        Category = "Redpoint EOS Online Framework",
        meta = (DisplayName = "Is Redpoint EOS Online Framework Free Edition?"))
    static bool IsRedpointFrameworkFreeEdition();

    /**
     * Returns if the target player controller is currently spectating.
     */
    UFUNCTION(
        BlueprintPure,
        Category = "Redpoint EOS Online Framework|Gameplay",
        meta = (DisplayName = "Is Player Controller Spectating?"))
    static bool IsPlayerControllerSpectating(APlayerController *PlayerController);

    /**
     * Sets whether the target player controller should be spectating.
     */
    UFUNCTION(
        BlueprintCallable,
        BlueprintAuthorityOnly,
        Category = "Redpoint EOS Online Framework|Gameplay",
        meta = (DisplayName = "Set Player Controller Spectating"))
    static void SetPlayerControllerSpectating(APlayerController *PlayerController, bool bIsSpectating);
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()