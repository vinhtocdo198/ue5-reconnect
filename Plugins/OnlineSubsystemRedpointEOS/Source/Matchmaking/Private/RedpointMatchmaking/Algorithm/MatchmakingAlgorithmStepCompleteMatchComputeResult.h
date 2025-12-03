// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStep.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMSTEP_COMPLETE_MATCH_COMPUTE_RESULT FName("CompleteMatchComputeResult")

class FMatchmakingAlgorithmStepStateCompleteMatchComputeResult : public IMatchmakingAlgorithmStepState
{
};

class FMatchmakingAlgorithmStepCompleteMatchComputeResult
    : public IMatchmakingAlgorithmStep<FMatchmakingAlgorithmStepStateCompleteMatchComputeResult>,
      public TSharedFromThis<FMatchmakingAlgorithmStepCompleteMatchComputeResult>
{
private:
    void OnLobbyUpdated(
        const FOnlineError &Error,
        const FUniqueNetId &UserId,
        TWeakPtr<FMatchmakingEngineTask> TaskWk,
        TArray<FMatchmakingEngineResponseTeam> Teams);

public:
    virtual ~FMatchmakingAlgorithmStepCompleteMatchComputeResult() override = default;
    UE_NONCOPYABLE(FMatchmakingAlgorithmStepCompleteMatchComputeResult);
    FMatchmakingAlgorithmStepCompleteMatchComputeResult() = default;

    virtual FName GetName() const override
    {
        return MMSTEP_COMPLETE_MATCH_COMPUTE_RESULT;
    }

    virtual FText GetStatus() const override
    {
        return NSLOCTEXT("Matchmaking", "CompleteMatchComputeResult_Status", "Finishing up");
    }

    virtual FText GetDetail() const override
    {
        return NSLOCTEXT("Matchmaking", "CompleteMatchComputeResult_Detail", "Calculating final teams...");
    }

    virtual TSharedPtr<FMatchmakingAlgorithmStepStateCompleteMatchComputeResult> CreateNewState() override;
    virtual void Start(TSharedRef<FMatchmakingEngineTask> Task) override;
};

#endif

REDPOINT_EOS_CODE_GUARD_END()