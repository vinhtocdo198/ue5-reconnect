// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "Brushes/SlateImageBrush.h"
#include "Fonts/SlateFontInfo.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointStartupScreen.h"
#include "Styling/SlateTypes.h"

#include "RedpointDefaultStartupScreen.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

UCLASS(BlueprintType, Category = "Redpoint EOS Online Framework")
class REDPOINTEOSFRAMEWORK_API URedpointDefaultStartupScreen : public UUserWidget, public IRedpointStartupScreen
{
    GENERATED_BODY()

public:
    URedpointDefaultStartupScreen(const FObjectInitializer &ObjectInitializer);

private:
    FButtonStyle ButtonStyle;
    FText CurrentMessage;
    bool bIsErrored;
    FOnStartupScreenStartLogin OnRetryLogin;
    TSharedPtr<FSlateImageBrush> LogoImage;

public:
    UPROPERTY()
    FSlateFontInfo MessageFont;

    UPROPERTY()
    FSlateFontInfo NoteFont;

    UPROPERTY()
    FSlateFontInfo RegularFont;

    virtual void OnStartupScreenDisplayed_Implementation(const FOnStartupScreenStartLogin &OnStartLogin) override;

    virtual void UpdateProgressMessage_Implementation(const FText &Message) override;

    virtual void OnLoginFailed_Implementation(const FText &LoginError, const FOnStartupScreenStartLogin &OnRetryLogin)
        override;

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;

private:
    EVisibility GetThrobberVisibility() const;
    FSlateColor GetMessageColor() const;
    FText GetMessageText() const;
    EVisibility GetRetryButtonVisibility() const;
    void OnRetryButtonPressed();
    FOptionalSize GetLogoImageRatio() const;
    FOptionalSize GetLogoImageDesiredWidth() const;
    const FSlateBrush *GetLogoImage() const;
    EVisibility GetLogoVisibility() const;
    EVisibility GetLogoNotConfiguredVisibility() const;
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()
