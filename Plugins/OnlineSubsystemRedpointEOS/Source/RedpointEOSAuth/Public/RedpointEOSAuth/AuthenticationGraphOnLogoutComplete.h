// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3236567328, Redpoint::EOS::Auth)
{
using namespace ::Redpoint::EOS::Auth;

typedef TDelegate<void()> FAuthenticationGraphOnLogoutComplete;

}

namespace Redpoint::EOS::Auth
{
REDPOINT_EOS_FILE_NS_EXPORT(3236567328, Redpoint::EOS::Auth, FAuthenticationGraphOnLogoutComplete)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()