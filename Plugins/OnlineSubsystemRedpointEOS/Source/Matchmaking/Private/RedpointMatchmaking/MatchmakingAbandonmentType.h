// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

enum class EMatchmakingAbandonmentType : uint8
{
    /**
     * Matchmaking will restart when abandonment is complete.
     */
    Restart,

    /**
     * Matchmaking will be cancelled when abandonment is complete.
     */
    Cancel,
};

#endif

REDPOINT_EOS_CODE_GUARD_END()