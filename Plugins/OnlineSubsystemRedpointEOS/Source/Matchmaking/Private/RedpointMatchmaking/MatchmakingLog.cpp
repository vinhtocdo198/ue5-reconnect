// Copyright June Rhodes. All Rights Reserved.

#include "RedpointMatchmaking/MatchmakingLog.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

DEFINE_LOG_CATEGORY(LogMatchmaking);
#if !REDPOINT_EOS_IS_FREE_EDITION
DEFINE_LOG_CATEGORY(LogMatchmakingSerialization);
DEFINE_LOG_CATEGORY(LogMatchmakingTeamManagement);
#endif

REDPOINT_EOS_CODE_GUARD_END()