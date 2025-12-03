// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "GameFramework/HUD.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointUMGHUD.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

/**
 * The default AHUD class is designed for immediate rendering and does not have it's BeginPlay or EndPlay events called
 * on clients. This makes it awkward to wire up UMG-based HUDs using AHUD, leaving most developers to manually control
 * the HUD directly in the player controller.
 *
 * This implementation of AHUD automatically adds the specified 'HUD UMG Widget' to the viewport when the HUD is
 * rendered for the first time, and appropriately hides it based on whether the HUD should currently be shown.
 */
UCLASS(BlueprintType, meta = (DisplayName = "Redpoint UMG HUD"))
class REDPOINTEOSFRAMEWORK_API ARedpointUMGHUD : public AHUD
{
    GENERATED_BODY()

public:
    /**
     * The UMG widget that will be displayed on the player's local screen.
     */
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Redpoint UMG HUD",
        meta = (DisplayName = "HUD UMG Widget"))
    TSubclassOf<UUserWidget> HUDUMGWidget;

    /**
     * The UMG widget instance that has been created for the HUD.
     */
    UPROPERTY(
        VisibleInstanceOnly,
        BlueprintReadOnly,
        Category = "Redpoint UMG HUD",
        meta = (DisplayName = "HUD UMG Widget Instance"))
    TObjectPtr<UUserWidget> HUDUMGWidgetInstance;

    /**
     * Called after the UMG widget has been shown - either during initial creation or because bShowHUD has been toggled
     * to true. You don't need to change the UMG widget's visibility; 'Redpoint UMG HUD' will change it for you.
     *
     * @param Widget The widget that is being displayed on the HUD.
     * @param bInitiallyCreated If true, this is after the widget has been created, and not because bShowHUD was toggled
     * to true.
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Redpoint UMG HUD")
    void PostWidgetShown(UUserWidget *Widget, bool bInitiallyCreated);

    /**
     * Called before the UMG widget has been hidden - either upon BeginDestroy or because bShowHUD has been toggled to
     * false. You don't need to change the UMG widget's visibility; 'Redpoint UMG HUD' will change it for you.
     *
     * @param Widget The widget that is about to be hidden or destroyed on the HUD.
     * @param bPendingDestruction If true, this is right before the widget is finally being destroyed in BeginDestroy
     * (it may already be hidden), and not because bShowHUD was toggled to false.
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Redpoint UMG HUD")
    void PreWidgetHidden(UUserWidget *Widget, bool bPendingDestruction);

    virtual void PostRender() override;

    virtual void BeginDestroy() override;
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()
