// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "EOSUserInterface_SignInOrCreateAccount_Choice.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2418376640, Redpoint::EOS::Auth::UserInterface)
{

typedef TDelegate<void()> FEOSUserInterface_EnterDevicePinCode_OnCancel;
typedef TDelegate<void(EEOSUserInterface_SignInOrCreateAccount_Choice Choice)>
    FEOSUserInterface_SignInOrCreateAccount_OnChoice;

}

namespace Redpoint::EOS::Auth::UserInterface
{
REDPOINT_EOS_FILE_NS_EXPORT(
    2418376640,
    Redpoint::EOS::Auth::UserInterface,
    FEOSUserInterface_EnterDevicePinCode_OnCancel)
REDPOINT_EOS_FILE_NS_EXPORT(
    2418376640,
    Redpoint::EOS::Auth::UserInterface,
    FEOSUserInterface_SignInOrCreateAccount_OnChoice)
}

REDPOINT_EOS_CODE_GUARD_END()