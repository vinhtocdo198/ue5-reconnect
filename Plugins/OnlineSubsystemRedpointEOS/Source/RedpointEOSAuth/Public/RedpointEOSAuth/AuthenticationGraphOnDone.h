// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAuth/AuthenticationGraphNodeResult.h"
#include "RedpointEOSAuth/AuthenticationGraphOnLogout.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(846847803, Redpoint::EOS::Auth)
{
using namespace ::Redpoint::EOS::Auth;

typedef TDelegate<void(EAuthenticationGraphNodeResult Result, const FAuthenticationGraphOnLogout &OnLogout)>
    FAuthenticationGraphOnDone;

}

namespace Redpoint::EOS::Auth
{
REDPOINT_EOS_FILE_NS_EXPORT(846847803, Redpoint::EOS::Auth, FAuthenticationGraphOnDone)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()