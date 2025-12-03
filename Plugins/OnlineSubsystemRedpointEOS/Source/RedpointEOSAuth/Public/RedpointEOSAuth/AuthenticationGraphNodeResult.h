// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1938579610, Redpoint::EOS::Auth)
{

enum class EAuthenticationGraphNodeResult : int8
{
    /* Continue the authentication process. */
    Continue,

    /* The authentication process encountered a permanent error, fail authentication. */
    Error,
};

}

namespace Redpoint::EOS::Auth
{
REDPOINT_EOS_FILE_NS_EXPORT(1938579610, Redpoint::EOS::Auth, EAuthenticationGraphNodeResult)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()