// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStep.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMSTEP_COMPLETE_MATCH FName("CompleteMatch")

class FMatchmakingAlgorithmStepStateCompleteMatch : public IMatchmakingAlgorithmStepState
{
};

class FMatchmakingAlgorithmStepCompleteMatch
    : public IMatchmakingAlgorithmStep<FMatchmakingAlgorithmStepStateCompleteMatch>,
      public TSharedFromThis<FMatchmakingAlgorithmStepCompleteMatch>
{
public:
    virtual ~FMatchmakingAlgorithmStepCompleteMatch() override = default;
    UE_NONCOPYABLE(FMatchmakingAlgorithmStepCompleteMatch);
    FMatchmakingAlgorithmStepCompleteMatch() = default;

    virtual FName GetName() const override
    {
        return MMSTEP_COMPLETE_MATCH;
    }

    virtual FText GetStatus() const override
    {
        return NSLOCTEXT("Matchmaking", "CompleteMatch_Status", "Finishing up");
    }

    virtual FText GetDetail() const override
    {
        return NSLOCTEXT("Matchmaking", "CompleteMatch_Detail", "Preparing to finish matchmaking...");
    }

    virtual TSharedPtr<FMatchmakingAlgorithmStepStateCompleteMatch> CreateNewState() override;
    virtual void Start(TSharedRef<FMatchmakingEngineTask> Task) override;
};

#endif

REDPOINT_EOS_CODE_GUARD_END()