// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "../MatchmakingAbandonmentType.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStep.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMSTEP_ABANDON_FINISH FName("AbandonFinish")

class FMatchmakingAlgorithmStepStateAbandonFinish : public IMatchmakingAlgorithmStepState
{
};

class FMatchmakingAlgorithmStepAbandonFinish
    : public IMatchmakingAlgorithmStep<FMatchmakingAlgorithmStepStateAbandonFinish, EMatchmakingAbandonmentType>,
      public TSharedFromThis<FMatchmakingAlgorithmStepAbandonFinish>
{
public:
    virtual ~FMatchmakingAlgorithmStepAbandonFinish() override = default;
    UE_NONCOPYABLE(FMatchmakingAlgorithmStepAbandonFinish);
    FMatchmakingAlgorithmStepAbandonFinish() = default;

    virtual FName GetName() const override
    {
        return MMSTEP_ABANDON_FINISH;
    }

    virtual FText GetStatus() const override
    {
        return NSLOCTEXT("Matchmaking", "AbandonFinish_Status", "Please wait");
    }

    virtual FText GetDetail() const override
    {
        return NSLOCTEXT("Matchmaking", "AbandonFinish_Detail", "Please wait...");
    }

    virtual TSharedPtr<FMatchmakingAlgorithmStepStateAbandonFinish> CreateNewState() override;
    virtual void Start(TSharedRef<FMatchmakingEngineTask> Task, EMatchmakingAbandonmentType AbandonmentType) override;

    virtual bool CanHandleInterrupt(const TSharedRef<FMatchmakingEngineTask> &Task, FName InterruptType) override;
    virtual bool HandleInterrupt(const TSharedRef<FMatchmakingEngineTask> &Task, FName InterruptType) override;
};

#endif

REDPOINT_EOS_CODE_GUARD_END()