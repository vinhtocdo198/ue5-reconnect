// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAuth/AuthenticationGraphOnLogoutComplete.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3884380495, Redpoint::EOS::Auth)
{
using namespace ::Redpoint::EOS::Auth;

typedef TDelegate<void(const FAuthenticationGraphOnLogoutComplete &OnComplete)> FAuthenticationGraphOnLogout;

}

namespace Redpoint::EOS::Auth
{
REDPOINT_EOS_FILE_NS_EXPORT(3884380495, Redpoint::EOS::Auth, FAuthenticationGraphOnLogout)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()