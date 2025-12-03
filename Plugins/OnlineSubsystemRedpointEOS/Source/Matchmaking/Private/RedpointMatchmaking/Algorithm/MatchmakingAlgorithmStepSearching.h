// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStep.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMSTEP_SEARCHING FName("Searching")

class FMatchmakingAlgorithmStepStateSearching : public IMatchmakingAlgorithmStepState
{
public:
    bool bInterruptedWithFollowRequest;
};

class FMatchmakingAlgorithmStepSearching
    : public IMatchmakingAlgorithmStep<FMatchmakingAlgorithmStepStateSearching, bool>,
      public TSharedFromThis<FMatchmakingAlgorithmStepSearching>
{
private:
    void OnDelayComplete(TWeakPtr<FMatchmakingEngineTask> TaskWk);

public:
    virtual ~FMatchmakingAlgorithmStepSearching() override = default;
    UE_NONCOPYABLE(FMatchmakingAlgorithmStepSearching);
    FMatchmakingAlgorithmStepSearching() = default;

    virtual FName GetName() const override
    {
        return MMSTEP_SEARCHING;
    }

    virtual FText GetStatus() const override
    {
        return NSLOCTEXT("Matchmaking", "Searching_Status", "Searching for other players");
    }

    virtual FText GetDetail() const override
    {
        return NSLOCTEXT("Matchmaking", "Searching_Detail", "Starting the search...");
    }

    virtual TSharedPtr<FMatchmakingAlgorithmStepStateSearching> CreateNewState() override;
    virtual void Start(TSharedRef<FMatchmakingEngineTask> Task, bool bIsInitialEntry) override;
    virtual bool CanHandleInterrupt(const TSharedRef<FMatchmakingEngineTask> &Task, FName InterruptType) override;
    virtual bool HandleInterrupt(const TSharedRef<FMatchmakingEngineTask> &Task, FName InterruptType) override;
};

#endif

REDPOINT_EOS_CODE_GUARD_END()