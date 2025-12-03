// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "MatchmakingCandidateEvaluator.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

class FMatchmakingCandidateEvaluatorDefault : public IMatchmakingCandidateEvaluator
{
public:
    virtual ~FMatchmakingCandidateEvaluatorDefault() override = default;

    virtual FMatchmakingSuitableMatchResult EvaluateCandidate(
        FMatchmakingEngineTask &Task,
        const FMatchmakingCandidateLobby &Candidate,
        double SecondsRemaining,
        double IterationMaximumSecondsRemaining) override;
};

#endif

REDPOINT_EOS_CODE_GUARD_END()