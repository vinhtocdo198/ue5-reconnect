// Copyright June Rhodes. All Rights Reserved.

#include "EOSDefaultUserInterface_EnterDevicePinCode.h"

#include "Blueprint/WidgetTree.h"
#include "Engine/Font.h"
#include "Styling/UMGCoreStyle.h"
#include "UObject/ConstructorHelpers.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBackgroundBlur.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "RedpointEOSAuth"

REDPOINT_EOS_CODE_GUARD_BEGIN()

UEOSDefaultUserInterface_EnterDevicePinCode::UEOSDefaultUserInterface_EnterDevicePinCode(
    const FObjectInitializer &ObjectInitializer)
    : UUserWidget(ObjectInitializer)
{
    if (!IsRunningDedicatedServer())
    {
        static ConstructorHelpers::FObjectFinder<UFont> RobotoFontObj(*UWidget::GetDefaultFontName());
        this->TitleFont = FSlateFontInfo(RobotoFontObj.Object, 24, FName("Bold"));
        this->RegularFont = FSlateFontInfo(RobotoFontObj.Object, 12, FName("Regular"));
    }

    this->ButtonStyle = FUMGCoreStyle::Get().GetWidgetStyle<FButtonStyle>("Button");
    this->ButtonStyle.UnlinkColors();

    this->VerificationUrl = "https://THISWILLBEREPLACED/";
    this->PinCode = "00000000";
}

FText UEOSDefaultUserInterface_EnterDevicePinCode::GetVerificationUrl() const
{
    return FText::FromString(this->VerificationUrl);
}

FText UEOSDefaultUserInterface_EnterDevicePinCode::GetPinCode() const
{
    return FText::FromString(this->PinCode);
}

void UEOSDefaultUserInterface_EnterDevicePinCode::OnCancel()
{
    if (IsValid(this->Context))
    {
        this->Context->CancelLogin();
    }
}

void UEOSDefaultUserInterface_EnterDevicePinCode::SetupUserInterface_Implementation(
    UEOSUserInterface_EnterDevicePinCode_Context *InContext,
    const FString &InVerificationUrl,
    const FString &InPinCode)
{
    this->Context = InContext;
    this->VerificationUrl = InVerificationUrl;
    this->PinCode = InPinCode;
}

TSharedRef<SWidget> UEOSDefaultUserInterface_EnterDevicePinCode::RebuildWidget()
{
    if (!this->WidgetTree)
    {
        this->WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"), RF_Transient);
    }

    auto Title = SNew(STextBlock)
                     .Font(this->TitleFont)
                     .ColorAndOpacity(FSlateColor(FColor::White))
                     .Text(LOCTEXT("EnterDevicePinCode_Title", "Enter PIN Code"));

    auto NavigatePrompt = SNew(STextBlock)
                              .Font(this->RegularFont)
                              .ColorAndOpacity(FSlateColor(FColor::White))
                              .Text(LOCTEXT(
                                  "EnterDevicePinCode_NavigatePrompt",
                                  "To finish signing into your account, open a web browser on your PC or phone and go "
                                  "to the following URL:"));

    auto NavigateValue = SNew(STextBlock)
                             .Font(this->TitleFont)
                             .ColorAndOpacity(FSlateColor(FColor::White))
                             .Text_UObject(this, &UEOSDefaultUserInterface_EnterDevicePinCode::GetVerificationUrl);

    auto EnterCodePrompt =
        SNew(STextBlock)
            .Font(this->RegularFont)
            .ColorAndOpacity(FSlateColor(FColor::White))
            .Text(LOCTEXT("EnterDevicePinCode_EnterCodePrompt", "Then enter the following PIN code:"));

    auto EnterCodeValue = SNew(STextBlock)
                              .Font(this->TitleFont)
                              .ColorAndOpacity(FSlateColor(FColor::White))
                              .Text_UObject(this, &UEOSDefaultUserInterface_EnterDevicePinCode::GetPinCode);

    auto CancelPrompt =
        SNew(STextBlock)
            .Font(this->RegularFont)
            .ColorAndOpacity(FSlateColor(FColor::White))
            .Text(LOCTEXT("EnterDevicePinCode_CancelPrompt", "If you no longer want to continue logging in:"));

    auto CancelButton = SNew(SButton)
                            .ButtonStyle(&this->ButtonStyle)
                            .ContentPadding(FMargin(10.0, 2.0))
                            .HAlign(EHorizontalAlignment::HAlign_Center)
                            .VAlign(EVerticalAlignment::VAlign_Center)
                            .OnPressed_UObject(this, &UEOSDefaultUserInterface_EnterDevicePinCode::OnCancel)
                                [SNew(STextBlock)
                                     .Font(this->RegularFont)
                                     .ColorAndOpacity(FSlateColor(FColor::Black))
                                     .Text(LOCTEXT("EnterDevicePinCode_CancelButtonText", "Cancel sign in"))];

    auto CancelButtonBox = SNew(SBox)
                               .MinDesiredWidth(140.0f)
                               .MinDesiredHeight(60.0f)
                               .Padding(0)
                               .HAlign(EHorizontalAlignment::HAlign_Fill)
                               .VAlign(EVerticalAlignment::VAlign_Fill)[CancelButton];

    auto LayoutBox = SNew(SVerticalBox) +
                     SVerticalBox::Slot()
                         .Padding(0)
                         .VAlign(EVerticalAlignment::VAlign_Fill)
                         .HAlign(EHorizontalAlignment::HAlign_Center)
                         .AutoHeight()[Title] +
                     SVerticalBox::Slot()
                         .Padding(0.0, 20.0, 0.0, 10.0)
                         .VAlign(EVerticalAlignment::VAlign_Fill)
                         .HAlign(EHorizontalAlignment::HAlign_Center)
                         .AutoHeight()[NavigatePrompt] +
                     SVerticalBox::Slot()
                         .Padding(0.0, 20.0, 0.0, 10.0)
                         .VAlign(EVerticalAlignment::VAlign_Fill)
                         .HAlign(EHorizontalAlignment::HAlign_Center)
                         .AutoHeight()[NavigateValue] +
                     SVerticalBox::Slot()
                         .Padding(0.0, 20.0, 0.0, 10.0)
                         .VAlign(EVerticalAlignment::VAlign_Fill)
                         .HAlign(EHorizontalAlignment::HAlign_Center)
                         .AutoHeight()[EnterCodePrompt] +
                     SVerticalBox::Slot()
                         .Padding(0.0, 20.0, 0.0, 10.0)
                         .VAlign(EVerticalAlignment::VAlign_Fill)
                         .HAlign(EHorizontalAlignment::HAlign_Center)
                         .AutoHeight()[EnterCodeValue] +
                     SVerticalBox::Slot()
                         .Padding(0.0, 20.0, 0.0, 10.0)
                         .VAlign(EVerticalAlignment::VAlign_Fill)
                         .HAlign(EHorizontalAlignment::HAlign_Center)
                         .AutoHeight()[CancelPrompt] +
                     SVerticalBox::Slot()
                         .Padding(10.0)
                         .VAlign(EVerticalAlignment::VAlign_Fill)
                         .HAlign(EHorizontalAlignment::HAlign_Center)
                         .AutoHeight()[CancelButtonBox];

    auto CenteringBox = SNew(SVerticalBox) + SVerticalBox::Slot().VAlign(EVerticalAlignment::VAlign_Center)[LayoutBox];

    auto BackgroundBlur = SNew(SBackgroundBlur).BlurStrength(10.0f)[CenteringBox];

    return BackgroundBlur;
}

#undef LOCTEXT_NAMESPACE

REDPOINT_EOS_CODE_GUARD_END()
