// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Logging/LogMacros.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace Redpoint::EOS::Presence
{

DECLARE_LOG_CATEGORY_EXTERN(LogRedpointEOSPresence, Verbose, All);

}

#endif

REDPOINT_EOS_CODE_GUARD_END()
