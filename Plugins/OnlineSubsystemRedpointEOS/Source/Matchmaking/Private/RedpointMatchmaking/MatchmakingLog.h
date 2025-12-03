// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

DECLARE_LOG_CATEGORY_EXTERN(LogMatchmaking, Verbose, Verbose);
#if !REDPOINT_EOS_IS_FREE_EDITION
DECLARE_LOG_CATEGORY_EXTERN(LogMatchmakingSerialization, Error, Verbose);
DECLARE_LOG_CATEGORY_EXTERN(LogMatchmakingTeamManagement, Warning, Verbose);
#endif

REDPOINT_EOS_CODE_GUARD_END()