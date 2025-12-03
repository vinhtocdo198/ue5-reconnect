// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Stats.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::Identity
{

REDPOINT_EOS_DECLARE_CYCLE_STAT(Identity, TEXT("Login"), Login);
REDPOINT_EOS_DECLARE_CYCLE_STAT(Identity, TEXT("Logout"), Logout);
REDPOINT_EOS_DECLARE_CYCLE_STAT(Identity, TEXT("UnexpectedLogout"), UnexpectedLogout);

}

REDPOINT_EOS_CODE_GUARD_END()
