// Copyright June Rhodes. All Rights Reserved.

#include "EOSDefaultUserInterface_SignInOrCreateAccount.h"

#include "Blueprint/WidgetTree.h"
#include "Engine/Font.h"
#include "Styling/UMGCoreStyle.h"
#include "UObject/ConstructorHelpers.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBackgroundBlur.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "RedpointEOSAuth"

REDPOINT_EOS_CODE_GUARD_BEGIN()

UEOSDefaultUserInterface_SignInOrCreateAccount::UEOSDefaultUserInterface_SignInOrCreateAccount(
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
}

void UEOSDefaultUserInterface_SignInOrCreateAccount::OnCreateAccount()
{
    if (IsValid(this->Context))
    {
        this->Context->SelectChoice(EEOSUserInterface_SignInOrCreateAccount_Choice::CreateAccount);
    }
}

void UEOSDefaultUserInterface_SignInOrCreateAccount::OnSignIn()
{
    if (IsValid(this->Context))
    {
        this->Context->SelectChoice(EEOSUserInterface_SignInOrCreateAccount_Choice::SignIn);
    }
}

void UEOSDefaultUserInterface_SignInOrCreateAccount::SetupUserInterface_Implementation(
    UEOSUserInterface_SignInOrCreateAccount_Context *InContext)
{
    this->Context = InContext;
}

TSharedRef<SWidget> UEOSDefaultUserInterface_SignInOrCreateAccount::RebuildWidget()
{
    if (!this->WidgetTree)
    {
        this->WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"), RF_Transient);
    }

    auto Title = SNew(STextBlock)
                     .Font(this->TitleFont)
                     .ColorAndOpacity(FSlateColor(FColor::White))
                     .Text(LOCTEXT("SignInOrCreateAccount_Title", "Create Account or Sign In"));

    auto Prompt = SNew(STextBlock)
                      .Font(this->RegularFont)
                      .ColorAndOpacity(FSlateColor(FColor::White))
                      .Text(LOCTEXT(
                          "SignInOrCreateAccount_Prompt",
                          "Is this your first time playing this game?\n\nIf it is, click Create Account.\n\nIf you've "
                          "played before, click Sign In."));

    auto CreateAccountButton =
        SNew(SBox)
            .MinDesiredWidth(140.0f)
            .MinDesiredHeight(60.0f)
            .Padding(0)
            .HAlign(EHorizontalAlignment::HAlign_Fill)
            .VAlign(EVerticalAlignment::VAlign_Fill)
                [SNew(SButton)
                     .ButtonStyle(&this->ButtonStyle)
                     .ContentPadding(FMargin(10.0, 2.0))
                     .HAlign(EHorizontalAlignment::HAlign_Center)
                     .VAlign(EVerticalAlignment::VAlign_Center)
                     .OnPressed_UObject(this, &UEOSDefaultUserInterface_SignInOrCreateAccount::OnCreateAccount)
                         [SNew(STextBlock)
                              .Font(this->RegularFont)
                              .ColorAndOpacity(FSlateColor(FColor::Black))
                              .Text(LOCTEXT("SignInOrCreateAccount_CreateAccount", "Create a new account"))]];

    auto SignInButton =
        SNew(SBox)
            .MinDesiredWidth(140.0f)
            .MinDesiredHeight(60.0f)
            .Padding(0)
            .HAlign(EHorizontalAlignment::HAlign_Fill)
            .VAlign(EVerticalAlignment::VAlign_Fill)
                [SNew(SButton)
                     .ButtonStyle(&this->ButtonStyle)
                     .ContentPadding(FMargin(10.0, 2.0))
                     .HAlign(EHorizontalAlignment::HAlign_Center)
                     .VAlign(EVerticalAlignment::VAlign_Center)
                     .OnPressed_UObject(this, &UEOSDefaultUserInterface_SignInOrCreateAccount::OnSignIn)
                         [SNew(STextBlock)
                              .Font(this->RegularFont)
                              .ColorAndOpacity(FSlateColor(FColor::Black))
                              .Text(LOCTEXT("SignInOrCreateAccount_SignIn", "Sign into an existing account"))]];

    auto ButtonList = SNew(SWrapBox)
                          .UseAllottedSize(true)
                          .HAlign(EHorizontalAlignment::HAlign_Left)
                          .Orientation(EOrientation::Orient_Horizontal)
                          .InnerSlotPadding(FVector2D(0.0, 0.0)) +
                      SWrapBox::Slot()
                          .Padding(10)
                          .HAlign(EHorizontalAlignment::HAlign_Fill)
                          .VAlign(EVerticalAlignment::VAlign_Fill)[CreateAccountButton] +
                      SWrapBox::Slot()
                          .Padding(10)
                          .HAlign(EHorizontalAlignment::HAlign_Fill)
                          .VAlign(EVerticalAlignment::VAlign_Fill)[SignInButton];

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
                         .AutoHeight()[Prompt] +
                     SVerticalBox::Slot()
                         .Padding(0.0, 10.0, 0.0, 0.0)
                         .VAlign(EVerticalAlignment::VAlign_Fill)
                         .HAlign(EHorizontalAlignment::HAlign_Center)
                         .FillHeight(1.0)[ButtonList];

    auto CenteringBox = SNew(SVerticalBox) + SVerticalBox::Slot().VAlign(EVerticalAlignment::VAlign_Center)[LayoutBox];

    auto BackgroundBlur = SNew(SBackgroundBlur).BlurStrength(10.0f)[CenteringBox];

    return BackgroundBlur;
}

#undef LOCTEXT_NAMESPACE

REDPOINT_EOS_CODE_GUARD_END()
