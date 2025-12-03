// Copyright June Rhodes. All Rights Reserved.

#include "RedpointDefaultStartupScreen.h"

#include "Blueprint/WidgetTree.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "RedpointGameInstance.h"
#include "Styling/UMGCoreStyle.h"
#include "UObject/ConstructorHelpers.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Images/SThrobber.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBackgroundBlur.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "RedpointEOSFramework"

REDPOINT_EOS_CODE_GUARD_BEGIN()

URedpointDefaultStartupScreen::URedpointDefaultStartupScreen(const FObjectInitializer &ObjectInitializer)
    : UUserWidget(ObjectInitializer)
    , CurrentMessage(LOCTEXT("StartupScreen_SigningIn", "Signing in..."))
{
    if (!IsRunningDedicatedServer())
    {
        static ConstructorHelpers::FObjectFinder<UFont> RobotoFontObj(*UWidget::GetDefaultFontName());
        this->MessageFont = FSlateFontInfo(RobotoFontObj.Object, 24, FName("Bold"));
        this->NoteFont = FSlateFontInfo(RobotoFontObj.Object, 20, FName("Regular"));
        this->RegularFont = FSlateFontInfo(RobotoFontObj.Object, 20, FName("Regular"));
    }

    // @note: The game instance will be our outer when constructed during startup. Otherwise try to get via world if
    // someone has loaded this UMG widget into the UMG editor.
    URedpointGameInstance *RedpointGameInstance = Cast<URedpointGameInstance>(this->GetOuter());
    if (!IsValid(RedpointGameInstance))
    {
        auto *World = this->GetWorld();
        if (IsValid(World))
        {
            RedpointGameInstance = Cast<URedpointGameInstance>(World->GetGameInstance());
        }
    }
    if (IsValid(RedpointGameInstance))
    {
        if (IsValid(RedpointGameInstance->StartupScreenLogo))
        {
            this->LogoImage = MakeShared<FSlateImageBrush>(
                RedpointGameInstance->StartupScreenLogo,
                RedpointGameInstance->StartupScreenLogoSize);
        }
    }

    this->ButtonStyle = FUMGCoreStyle::Get().GetWidgetStyle<FButtonStyle>("Button");
    this->ButtonStyle.UnlinkColors();
}

void URedpointDefaultStartupScreen::OnStartupScreenDisplayed_Implementation(
    const FOnStartupScreenStartLogin &OnStartLogin)
{
    OnStartLogin.ExecuteIfBound();
}

void URedpointDefaultStartupScreen::UpdateProgressMessage_Implementation(const FText &Message)
{
    this->CurrentMessage = Message;
}

void URedpointDefaultStartupScreen::OnLoginFailed_Implementation(
    const FText &LoginError,
    const FOnStartupScreenStartLogin &InOnRetryLogin)
{
    this->OnRetryLogin = InOnRetryLogin;
    this->CurrentMessage = LoginError;
    this->bIsErrored = true;
}

TSharedRef<SWidget> URedpointDefaultStartupScreen::RebuildWidget()
{
    if (!this->WidgetTree)
    {
        this->WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"), RF_Transient);
    }

    auto Logo = SNew(SBox)
                    .Visibility_UObject(this, &URedpointDefaultStartupScreen::GetLogoVisibility)
                    .MinAspectRatio_UObject(this, &URedpointDefaultStartupScreen::GetLogoImageRatio)
                    .MaxAspectRatio_UObject(this, &URedpointDefaultStartupScreen::GetLogoImageRatio)
                    .MinDesiredWidth_UObject(this, &URedpointDefaultStartupScreen::GetLogoImageDesiredWidth)
                        [SNew(SImage).Image_UObject(this, &URedpointDefaultStartupScreen::GetLogoImage)];

    auto LogoNotConfigured =
        SNew(SBox)
            .Visibility_UObject(this, &URedpointDefaultStartupScreen::GetLogoNotConfiguredVisibility)
            .Padding(40.f, 0.f)
                [SNew(STextBlock)
                     .Font(this->NoteFont)
                     .ColorAndOpacity(FSlateColor(FColor::White))
                     .Justification(ETextJustify::Center)
                     .AutoWrapText(true)
                     .Text(LOCTEXT(
                         "StartupScreen_NotConfigured",
                         "You can configure a logo to display here by creating a subclass of URedpointGameInstance, "
                         "setting the Startup Screen Logo property, and then setting your new subclass as the game "
                         "instance in Project Settings. This message will not display in Shipping builds."))];

    auto Spinner = SNew(SBox)[SNew(SThrobber).NumPieces(8)].Visibility_UObject(
        this,
        &URedpointDefaultStartupScreen::GetThrobberVisibility);

    auto Message = SNew(STextBlock)
                       .Font(this->MessageFont)
                       .ColorAndOpacity_UObject(this, &URedpointDefaultStartupScreen::GetMessageColor)
                       .Text_UObject(this, &URedpointDefaultStartupScreen::GetMessageText);

    auto RetryButton = SNew(SBox)
                           .MinDesiredWidth(140.0f)
                           .MinDesiredHeight(60.0f)
                           .Padding(0)
                           .HAlign(EHorizontalAlignment::HAlign_Fill)
                           .VAlign(EVerticalAlignment::VAlign_Fill)
                           .Visibility_UObject(this, &URedpointDefaultStartupScreen::GetRetryButtonVisibility)
                               [SNew(SButton)
                                    .ButtonStyle(&this->ButtonStyle)
                                    .ContentPadding(FMargin(10.0, 2.0))
                                    .HAlign(EHorizontalAlignment::HAlign_Center)
                                    .VAlign(EVerticalAlignment::VAlign_Center)
                                    .OnPressed_UObject(this, &URedpointDefaultStartupScreen::OnRetryButtonPressed)
                                        [SNew(STextBlock)
                                             .Font(this->RegularFont)
                                             .ColorAndOpacity(FSlateColor(FColor::Black))
                                             .Text(LOCTEXT("StartupScreen_RetryLogin", "Retry Login"))]];

    auto VerticalBox = SNew(SVerticalBox) +
                       SVerticalBox::Slot()
                           .HAlign(EHorizontalAlignment::HAlign_Center)
                           .AutoHeight()
                           .Padding(0.f, 0.f, 0.f, 20.f)[Spinner] +
                       SVerticalBox::Slot().HAlign(EHorizontalAlignment::HAlign_Center).AutoHeight()[Message] +
                       SVerticalBox::Slot()
                           .HAlign(EHorizontalAlignment::HAlign_Center)
                           .AutoHeight()
                           .Padding(0.f, 20.f, 0.f, 0.f)[RetryButton];

    auto LayoutBox =
        SNew(SVerticalBox) + SVerticalBox::Slot().FillHeight(0.25f) +
        SVerticalBox::Slot().FillHeight(0.5f).VAlign(EVerticalAlignment::VAlign_Center)[Logo] +
        SVerticalBox::Slot().FillHeight(0.5f).VAlign(EVerticalAlignment::VAlign_Center)[LogoNotConfigured] +
        SVerticalBox::Slot().FillHeight(0.05f) +
        SVerticalBox::Slot().FillHeight(0.15f).VAlign(EVerticalAlignment::VAlign_Center)[VerticalBox] +
        SVerticalBox::Slot().FillHeight(0.05f);

    return LayoutBox;
}

EVisibility URedpointDefaultStartupScreen::GetThrobberVisibility() const
{
    return !this->bIsErrored ? EVisibility::Visible : EVisibility::Collapsed;
}

FSlateColor URedpointDefaultStartupScreen::GetMessageColor() const
{
    return FSlateColor(bIsErrored ? FColor::Red : FColor::White);
}

FText URedpointDefaultStartupScreen::GetMessageText() const
{
    return this->CurrentMessage;
}

EVisibility URedpointDefaultStartupScreen::GetRetryButtonVisibility() const
{
    return this->bIsErrored ? EVisibility::Visible : EVisibility::Collapsed;
}

void URedpointDefaultStartupScreen::OnRetryButtonPressed()
{
    this->bIsErrored = false;
    this->CurrentMessage = LOCTEXT("StartupScreen_SigningIn", "Signing in...");
    this->OnRetryLogin.ExecuteIfBound();
}

FOptionalSize URedpointDefaultStartupScreen::GetLogoImageRatio() const
{
    if (!this->LogoImage.IsValid())
    {
        return 1.0f;
    }

    return this->LogoImage->GetImageSize().X / this->LogoImage->GetImageSize().Y;
}

FOptionalSize URedpointDefaultStartupScreen::GetLogoImageDesiredWidth() const
{
    if (!this->LogoImage.IsValid())
    {
        return 0.0f;
    }

    return this->LogoImage->GetImageSize().X;
}

const FSlateBrush *URedpointDefaultStartupScreen::GetLogoImage() const
{
    return this->LogoImage.Get();
}

EVisibility URedpointDefaultStartupScreen::GetLogoVisibility() const
{
    return this->LogoImage.IsValid() ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility URedpointDefaultStartupScreen::GetLogoNotConfiguredVisibility() const
{
    return this->LogoImage.IsValid() ? EVisibility::Collapsed :
#if UE_BUILD_SHIPPING
                                     EVisibility::Hidden
#else
                                     EVisibility::Visible
#endif
        ;
}

#undef LOCTEXT_NAMESPACE

REDPOINT_EOS_CODE_GUARD_END()
