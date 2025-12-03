// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAuth/UserInterface/UserInterfaceDelegates.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

#include "EOSUserInterface_EnterDevicePinCode_Context.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

UCLASS(BlueprintType) class REDPOINTEOSAUTH_API UEOSUserInterface_EnterDevicePinCode_Context : public UObject
{
    GENERATED_BODY()

private:
    Redpoint::EOS::Auth::UserInterface::FEOSUserInterface_EnterDevicePinCode_OnCancel OnCancel;

public:
    void SetOnCancel(Redpoint::EOS::Auth::UserInterface::FEOSUserInterface_EnterDevicePinCode_OnCancel InOnCancel);

    UFUNCTION(BlueprintCallable, Category = "EOS|User Interface")
    virtual void CancelLogin();
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()
