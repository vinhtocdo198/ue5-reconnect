// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "EOSUserInterface_SignInOrCreateAccount.h"
#include "Fonts/SlateFontInfo.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "Styling/SlateTypes.h"

#include "EOSDefaultUserInterface_SignInOrCreateAccount.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

UCLASS(BlueprintType, Category = "Redpoint EOS Online Framework")
class REDPOINTEOSAUTH_API UEOSDefaultUserInterface_SignInOrCreateAccount
    : public UUserWidget,
      public IEOSUserInterface_SignInOrCreateAccount
{
    GENERATED_BODY()

public:
    UEOSDefaultUserInterface_SignInOrCreateAccount(const FObjectInitializer &ObjectInitializer);

private:
    FButtonStyle ButtonStyle;

    void OnCreateAccount();
    void OnSignIn();

public:
    UPROPERTY()
    FSlateFontInfo TitleFont;

    UPROPERTY()
    FSlateFontInfo RegularFont;

    UPROPERTY()
    TObjectPtr<UEOSUserInterface_SignInOrCreateAccount_Context> Context;

    virtual void SetupUserInterface_Implementation(UEOSUserInterface_SignInOrCreateAccount_Context *InContext) override;

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()
