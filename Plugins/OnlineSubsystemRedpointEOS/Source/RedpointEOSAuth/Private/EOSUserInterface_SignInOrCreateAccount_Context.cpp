// Copyright June Rhodes. All Rights Reserved.

#include "EOSUserInterface_SignInOrCreateAccount_Context.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

void UEOSUserInterface_SignInOrCreateAccount_Context::SetOnChoice(
    Redpoint::EOS::Auth::UserInterface::FEOSUserInterface_SignInOrCreateAccount_OnChoice InOnChoice)
{
    this->OnChoice = MoveTemp(InOnChoice);
}

void UEOSUserInterface_SignInOrCreateAccount_Context::SelectChoice(
    EEOSUserInterface_SignInOrCreateAccount_Choice SelectedChoice)
{
    this->OnChoice.ExecuteIfBound(SelectedChoice);
}

REDPOINT_EOS_CODE_GUARD_END()
