// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

#include "EOSUserInterface_SignInOrCreateAccount_Choice.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

UENUM(BlueprintType)
enum class EEOSUserInterface_SignInOrCreateAccount_Choice : uint8
{
    /** The user wants to sign in with an existing account. */
    SignIn,

    /** The user wants to create an account. */
    CreateAccount
};

REDPOINT_EOS_CODE_GUARD_END()
