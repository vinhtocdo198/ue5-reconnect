// Copyright June Rhodes. All Rights Reserved.

#include "RedpointUMGHUD.h"

#include "Engine/GameInstance.h"
#include "LogRedpointEOSFramework.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

void ARedpointUMGHUD::PostRender()
{
    APlayerController *PC = this->GetOwningPlayerController();
    if (!IsValid(PC))
    {
        return;
    }

    UWorld *World = PC->GetWorld();
    if (!IsValid(World))
    {
        return;
    }

    UGameInstance *GameInstance = World->GetGameInstance();
    if (!IsValid(GameInstance))
    {
        return;
    }

    if (this->bShowHUD && IsValid(this->HUDUMGWidget))
    {
        bool bInitialCreated = false;

        if (!this->HUDUMGWidgetInstance)
        {
            this->HUDUMGWidgetInstance =
                NewObject<UUserWidget>(GameInstance, this->HUDUMGWidget, NAME_None, RF_Transactional);
            if (this->HUDUMGWidgetInstance)
            {
                this->HUDUMGWidgetInstance->SetOwningPlayer(PC);
                this->HUDUMGWidgetInstance->AddToPlayerScreen(0);
                bInitialCreated = true;
                UE_LOG(
                    LogRedpointEOSFramework,
                    VeryVerbose,
                    TEXT("Created UMG widget for Redpoint UMG HUD and associated it with the owning player."));
            }
        }

        if (this->HUDUMGWidgetInstance &&
            this->HUDUMGWidgetInstance->GetVisibility() != ESlateVisibility::SelfHitTestInvisible)
        {
            this->HUDUMGWidgetInstance->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
            UE_LOG(
                LogRedpointEOSFramework,
                VeryVerbose,
                TEXT("Set visibility of UMG widget for Redpoint UMG HUD to visible (but not self hit-testable)."));
            this->PostWidgetShown(this->HUDUMGWidgetInstance, bInitialCreated);
        }
        else if (this->HUDUMGWidgetInstance && bInitialCreated)
        {
            // In case the widget is already set to SelfHitTestInvisible on creation.
            this->PostWidgetShown(this->HUDUMGWidgetInstance, bInitialCreated);
        }
    }
    else
    {
        if (this->HUDUMGWidgetInstance && this->HUDUMGWidgetInstance->GetVisibility() != ESlateVisibility::Collapsed)
        {
            this->PreWidgetHidden(this->HUDUMGWidgetInstance, false);
            this->HUDUMGWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
            UE_LOG(
                LogRedpointEOSFramework,
                VeryVerbose,
                TEXT("Set visibility of UMG widget for Redpoint UMG HUD to collapsed."));
        }
    }

    Super::PostRender();
}

void ARedpointUMGHUD::BeginDestroy()
{
    if (this->HUDUMGWidgetInstance)
    {
        this->PreWidgetHidden(this->HUDUMGWidgetInstance, true);
        this->HUDUMGWidgetInstance->RemoveFromParent();
        this->HUDUMGWidgetInstance = nullptr;
    }

    Super::BeginDestroy();
}

void ARedpointUMGHUD::PostWidgetShown_Implementation(UUserWidget *, bool)
{
}

void ARedpointUMGHUD::PreWidgetHidden_Implementation(UUserWidget *, bool)
{
}

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()
