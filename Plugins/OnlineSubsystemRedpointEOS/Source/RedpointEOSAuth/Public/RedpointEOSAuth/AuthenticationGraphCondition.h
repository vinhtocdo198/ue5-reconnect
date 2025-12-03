// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAuth/AuthenticationGraphState.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(878493240, Redpoint::EOS::Auth)
{
using namespace ::Redpoint::EOS::Auth;

typedef TDelegate<bool(const FAuthenticationGraphState &State)> FAuthenticationGraphCondition;

}

namespace Redpoint::EOS::Auth
{
REDPOINT_EOS_FILE_NS_EXPORT(878493240, Redpoint::EOS::Auth, FAuthenticationGraphCondition)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()