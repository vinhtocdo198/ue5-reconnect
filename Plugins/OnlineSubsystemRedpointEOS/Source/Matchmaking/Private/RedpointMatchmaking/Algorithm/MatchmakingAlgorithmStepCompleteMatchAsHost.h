// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStep.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMSTEP_COMPLETE_MATCH_AS_HOST FName("CompleteMatchAsHost")

class FMatchmakingAlgorithmStepStateCompleteMatchAsHost : public IMatchmakingAlgorithmStepState
{
};

class FMatchmakingAlgorithmStepCompleteMatchAsHost
    : public IMatchmakingAlgorithmStep<FMatchmakingAlgorithmStepStateCompleteMatchAsHost>,
      public TSharedFromThis<FMatchmakingAlgorithmStepCompleteMatchAsHost>
{
public:
    virtual ~FMatchmakingAlgorithmStepCompleteMatchAsHost() override = default;
    UE_NONCOPYABLE(FMatchmakingAlgorithmStepCompleteMatchAsHost);
    FMatchmakingAlgorithmStepCompleteMatchAsHost() = default;

    virtual FName GetName() const override
    {
        return MMSTEP_COMPLETE_MATCH_AS_HOST;
    }

    virtual FText GetStatus() const override
    {
        return NSLOCTEXT("Matchmaking", "CompleteMatchAsHost_Status", "Finishing up");
    }

    virtual FText GetDetail() const override
    {
        return NSLOCTEXT("Matchmaking", "CompleteMatchAsHost_Detail", "Preparing to finish matchmaking...");
    }

    virtual TSharedPtr<FMatchmakingAlgorithmStepStateCompleteMatchAsHost> CreateNewState() override;
    virtual void Start(TSharedRef<FMatchmakingEngineTask> Task) override;
};

#endif

REDPOINT_EOS_CODE_GUARD_END()