// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "EOSUserInterface_SignInOrCreateAccount_Choice.h"
#include "RedpointEOSAuth/UserInterface/UserInterfaceDelegates.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

#include "EOSUserInterface_SignInOrCreateAccount_Context.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

UCLASS(BlueprintType)
class REDPOINTEOSAUTH_API UEOSUserInterface_SignInOrCreateAccount_Context : public UObject
{
    GENERATED_BODY()

private:
    Redpoint::EOS::Auth::UserInterface::FEOSUserInterface_SignInOrCreateAccount_OnChoice OnChoice;

public:
    void SetOnChoice(Redpoint::EOS::Auth::UserInterface::FEOSUserInterface_SignInOrCreateAccount_OnChoice InOnChoice);

    UFUNCTION(BlueprintCallable, Category = "EOS|User Interface")
    virtual void SelectChoice(EEOSUserInterface_SignInOrCreateAccount_Choice SelectedChoice);
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()
