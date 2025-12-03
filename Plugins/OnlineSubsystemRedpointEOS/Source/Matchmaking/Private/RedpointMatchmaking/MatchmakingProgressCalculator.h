// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/MatchmakingEngine.h"
#include "RedpointMatchmaking/MatchmakingEngineTask.h"
#include "Templates/SharedPointer.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

class FMatchmakingProgressCalculator : public TSharedFromThis<FMatchmakingProgressCalculator>
{
public:
    UE_NONCOPYABLE(FMatchmakingProgressCalculator);

    static float ComputeProgress(const FMatchmakingEngineTask &InTask);
};

#endif

REDPOINT_EOS_CODE_GUARD_END()