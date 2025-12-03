// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "../MatchmakingCandidateEvaluator.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStep.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMSTEP_SEARCHING_SEND_FOLLOW_REQUEST FName("SearchingSendFollowRequest")

class FMatchmakingAlgorithmStepStateSearchingSendFollowRequest : public IMatchmakingAlgorithmStepState
{
};

class FMatchmakingAlgorithmStepSearchingSendFollowRequest
    : public IMatchmakingAlgorithmStep<
          FMatchmakingAlgorithmStepStateSearchingSendFollowRequest,
          FMatchmakingSuitableMatchResult>,
      public TSharedFromThis<FMatchmakingAlgorithmStepSearchingSendFollowRequest>
{
private:
public:
    virtual ~FMatchmakingAlgorithmStepSearchingSendFollowRequest() override = default;
    UE_NONCOPYABLE(FMatchmakingAlgorithmStepSearchingSendFollowRequest);
    FMatchmakingAlgorithmStepSearchingSendFollowRequest() = default;

    virtual FName GetName() const override
    {
        return MMSTEP_SEARCHING_SEND_FOLLOW_REQUEST;
    }

    virtual FText GetStatus() const override
    {
        return NSLOCTEXT("Matchmaking", "SearchingSendFollowRequest_Status", "Searching for other players");
    }

    virtual FText GetDetail() const override
    {
        return NSLOCTEXT("Matchmaking", "SearchingSendFollowRequest_Detail", "Communicating with other players...");
    }

    virtual TSharedPtr<FMatchmakingAlgorithmStepStateSearchingSendFollowRequest> CreateNewState() override;
    virtual void Start(TSharedRef<FMatchmakingEngineTask> Task, FMatchmakingSuitableMatchResult MatchCandidate)
        override;
};

#endif

REDPOINT_EOS_CODE_GUARD_END()