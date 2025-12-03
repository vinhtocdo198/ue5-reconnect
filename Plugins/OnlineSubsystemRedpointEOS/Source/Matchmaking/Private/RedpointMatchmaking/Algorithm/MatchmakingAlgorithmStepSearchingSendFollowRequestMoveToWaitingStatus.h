// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "../MatchmakingCandidateEvaluator.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStep.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMSTEP_SEARCHING_SEND_FOLLOW_REQUEST_MOVE_TO_WAITING_STATUS                                                    \
    FName("SearchingSendFollowRequestMoveToWaitingStatus")

class FMatchmakingAlgorithmStepStateSearchingSendFollowRequestMoveToWaitingStatus
    : public IMatchmakingAlgorithmStepState
{
};

class FMatchmakingAlgorithmStepSearchingSendFollowRequestMoveToWaitingStatus
    : public IMatchmakingAlgorithmStep<
          FMatchmakingAlgorithmStepStateSearchingSendFollowRequestMoveToWaitingStatus,
          FMatchmakingSuitableMatchResult>,
      public TSharedFromThis<FMatchmakingAlgorithmStepSearchingSendFollowRequestMoveToWaitingStatus>
{
private:
    void OnLobbyUpdated(
        const FOnlineError &Error,
        const FUniqueNetId &UserId,
        TWeakPtr<FMatchmakingEngineTask> TaskWk,
        FMatchmakingSuitableMatchResult MatchCandidate);

public:
    virtual ~FMatchmakingAlgorithmStepSearchingSendFollowRequestMoveToWaitingStatus() override = default;
    UE_NONCOPYABLE(FMatchmakingAlgorithmStepSearchingSendFollowRequestMoveToWaitingStatus);
    FMatchmakingAlgorithmStepSearchingSendFollowRequestMoveToWaitingStatus() = default;

    virtual FName GetName() const override
    {
        return MMSTEP_SEARCHING_SEND_FOLLOW_REQUEST_MOVE_TO_WAITING_STATUS;
    }

    virtual FText GetStatus() const override
    {
        return NSLOCTEXT(
            "Matchmaking",
            "SearchingSendFollowRequestMoveToWaitingStatus_Status",
            "Searching for other players");
    }

    virtual FText GetDetail() const override
    {
        return NSLOCTEXT(
            "Matchmaking",
            "SearchingSendFollowRequestMoveToWaitingStatus_Detail",
            "Communicating with other players...");
    }

    virtual TSharedPtr<FMatchmakingAlgorithmStepStateSearchingSendFollowRequestMoveToWaitingStatus> CreateNewState()
        override;
    virtual void Start(TSharedRef<FMatchmakingEngineTask> Task, FMatchmakingSuitableMatchResult MatchCandidate)
        override;
};

#endif

REDPOINT_EOS_CODE_GUARD_END()