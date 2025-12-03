// Copyright June Rhodes. All Rights Reserved.

#include "RedpointAvatar.h"

#include "Blueprint/WidgetTree.h"
#include "Engine/GameInstance.h"
#include "RedpointEOSFramework/SSquareBox.h"
#include "RedpointFrameworkSubsystem.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Images/SThrobber.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/SBoxPanel.h"

#define LOCTEXT_NAMESPACE "RedpointEOSFramework"

REDPOINT_EOS_CODE_GUARD_BEGIN()

URedpointAvatar::URedpointAvatar(const FObjectInitializer &ObjectInitializer)
    : UUserWidget(ObjectInitializer)
    , TargetUserId()
    , DefaultTexture()
    , LoadingIndicator(ERedpointAvatarLoadingIndicator::Throbber)
    , ExpirationMinutes(10.0)
    , CurrentImage()
    , bCurrentIsLoading(true)
    , bCurrentIsFailed(false)
    , PreviousTargetUserId()
{
}

TSharedRef<SWidget> URedpointAvatar::RebuildWidget()
{
    using namespace ::Redpoint::EOS::Framework;

    if (!this->WidgetTree)
    {
        this->WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"), RF_Transient);
    }

    auto Image = SNew(SImage).Image_UObject(this, &URedpointAvatar::GetImage);

    auto Throbber =
        SNew(SSquareBox)
            .Axis(ERedpointSquareBoxAxis::Vertical)[SNew(SScaleBox).Stretch(EStretch::ScaleToFit)[SNew(SThrobber)]];

    auto Layout = SNew(SWidgetSwitcher).WidgetIndex_UObject(this, &URedpointAvatar::GetWidgetSwitcherIndex) +
                  SWidgetSwitcher::Slot()
                      .HAlign(EHorizontalAlignment::HAlign_Fill)
                      .VAlign(EVerticalAlignment::VAlign_Fill)[Image] +
                  SWidgetSwitcher::Slot()
                      .HAlign(EHorizontalAlignment::HAlign_Fill)
                      .VAlign(EVerticalAlignment::VAlign_Fill)[Throbber] +
                  SWidgetSwitcher::Slot();

    return Layout;
}

const FSlateBrush *URedpointAvatar::GetImage() const
{
    return &this->CurrentImage;
}

int URedpointAvatar::GetWidgetSwitcherIndex() const
{
    if (!this->bCurrentIsLoading)
    {
        // Always display image if we're not loading.
        return 0;
    }

    switch (this->LoadingIndicator)
    {
    case ERedpointAvatarLoadingIndicator::DefaultTexture:
        return 0;
    case ERedpointAvatarLoadingIndicator::Throbber:
        return 1;
    case ERedpointAvatarLoadingIndicator::None:
    default:
        return 2;
    }
}

void URedpointAvatar::NativeTick(const FGeometry &MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    auto *World = this->GetWorld();
    if (World)
    {
        auto *GameInstance = World->GetGameInstance();
        if (GameInstance)
        {
            auto *FrameworkSubsystem = GameInstance->GetSubsystem<URedpointFrameworkSubsystem>();
            if (FrameworkSubsystem)
            {
                auto *LocalPlayer = this->GetOwningLocalPlayer();
                if (LocalPlayer)
                {
                    auto LocalUserId = LocalPlayer->GetCachedUniqueNetId();
                    if (LocalUserId.IsValid())
                    {
                        auto ResolvedTargetUserId = this->TargetUserId;
                        if (this->bTargetUserIsLocalUser)
                        {
                            ResolvedTargetUserId = LocalUserId;
                        }

                        if (ResolvedTargetUserId.IsValid())
                        {
                            if (this->PreviousTargetUserId != ResolvedTargetUserId)
                            {
                                // Make sure we clear the texture immediately if the user ID is ever changed.
                                this->CurrentImage.SetResourceObject(nullptr);
                                this->PreviousTargetUserId = ResolvedTargetUserId;
                                this->bCurrentIsLoading = true;
                                this->bCurrentIsFailed = false;
                            }

                            auto *NewTexture = FrameworkSubsystem->FetchAvatar(
                                LocalUserId,
                                ResolvedTargetUserId,
                                FTimespan::FromMinutes(this->ExpirationMinutes),
                                this->bCurrentIsFailed);
                            if (IsValid(NewTexture))
                            {
                                this->CurrentImage.SetResourceObject(NewTexture);
                                this->CurrentImage.SetImageSize(
                                    FVector2f(NewTexture->GetSurfaceWidth(), NewTexture->GetSurfaceHeight()));
                                this->bCurrentIsLoading = false;
                                this->bCurrentIsFailed = false;
                            }
                            else
                            {
                                this->bCurrentIsLoading = !this->bCurrentIsFailed;
                            }
                        }
                    }
                }
            }
        }
    }

    if (IsValid(this->DefaultTexture) && !IsValid(this->CurrentImage.GetResourceObject()) &&
        (this->bCurrentIsLoading || this->bCurrentIsFailed))
    {
        this->CurrentImage.SetResourceObject(this->DefaultTexture);
        this->CurrentImage.SetImageSize(
            FVector2f(this->DefaultTexture->GetSurfaceWidth(), this->DefaultTexture->GetSurfaceHeight()));
    }
}

#undef LOCTEXT_NAMESPACE

REDPOINT_EOS_CODE_GUARD_END()
