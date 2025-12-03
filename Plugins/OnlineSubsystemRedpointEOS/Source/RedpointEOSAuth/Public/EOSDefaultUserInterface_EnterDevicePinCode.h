// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "EOSUserInterface_EnterDevicePinCode.h"
#include "Fonts/SlateFontInfo.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "Styling/SlateTypes.h"

#include "EOSDefaultUserInterface_EnterDevicePinCode.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

UCLASS(BlueprintType, Category = "Redpoint EOS Online Framework")
class REDPOINTEOSAUTH_API UEOSDefaultUserInterface_EnterDevicePinCode : public UUserWidget,
                                                                        public IEOSUserInterface_EnterDevicePinCode
{
    GENERATED_BODY()

public:
    UEOSDefaultUserInterface_EnterDevicePinCode(const FObjectInitializer &ObjectInitializer);

private:
    FButtonStyle ButtonStyle;

    FString VerificationUrl;
    FString PinCode;

    FText GetVerificationUrl() const;
    FText GetPinCode() const;
    void OnCancel();

public:
    UPROPERTY()
    FSlateFontInfo TitleFont;

    UPROPERTY()
    FSlateFontInfo RegularFont;

    UPROPERTY()
    TObjectPtr<UEOSUserInterface_EnterDevicePinCode_Context> Context;

    void SetupUserInterface_Implementation(
        UEOSUserInterface_EnterDevicePinCode_Context *InContext,
        const FString &InVerificationUrl,
        const FString &InPinCode) override;

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()
