// Copyright June Rhodes. All Rights Reserved.

#include "EOSUserInterface_EnterDevicePinCode_Context.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

void UEOSUserInterface_EnterDevicePinCode_Context::SetOnCancel(
    Redpoint::EOS::Auth::UserInterface::FEOSUserInterface_EnterDevicePinCode_OnCancel InOnCancel)
{
    this->OnCancel = MoveTemp(InOnCancel);
}

UFUNCTION(BlueprintCallable, Category = "EOS|User Interface")
void UEOSUserInterface_EnterDevicePinCode_Context::CancelLogin()
{
    this->OnCancel.ExecuteIfBound();
}

REDPOINT_EOS_CODE_GUARD_END()
