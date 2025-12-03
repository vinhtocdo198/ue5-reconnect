// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "../MatchmakingAbandonmentType.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStep.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMSTEP_ABANDON_START FName("AbandonStart")

class FMatchmakingAlgorithmStepStateAbandonStart : public IMatchmakingAlgorithmStepState
{
};

class FMatchmakingAlgorithmStepAbandonStart
    : public IMatchmakingAlgorithmStep<FMatchmakingAlgorithmStepStateAbandonStart, EMatchmakingAbandonmentType>,
      public TSharedFromThis<FMatchmakingAlgorithmStepAbandonStart>
{
public:
    virtual ~FMatchmakingAlgorithmStepAbandonStart() override = default;
    UE_NONCOPYABLE(FMatchmakingAlgorithmStepAbandonStart);
    FMatchmakingAlgorithmStepAbandonStart() = default;

    virtual FName GetName() const override
    {
        return MMSTEP_ABANDON_START;
    }

    virtual FText GetStatus() const override
    {
        return NSLOCTEXT("Matchmaking", "AbandonStart_Status", "Please wait");
    }

    virtual FText GetDetail() const override
    {
        return NSLOCTEXT("Matchmaking", "AbandonStart_Detail", "Please wait...");
    }

    virtual TSharedPtr<FMatchmakingAlgorithmStepStateAbandonStart> CreateNewState() override;
    virtual void Start(TSharedRef<FMatchmakingEngineTask> Task, EMatchmakingAbandonmentType AbandonmentType) override;

    virtual bool CanHandleInterrupt(const TSharedRef<FMatchmakingEngineTask> &Task, FName InterruptType) override;
    virtual bool HandleInterrupt(const TSharedRef<FMatchmakingEngineTask> &Task, FName InterruptType) override;
};

#endif

REDPOINT_EOS_CODE_GUARD_END()