// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStep.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

class FMatchmakingAlgorithmStepStateError : public IMatchmakingAlgorithmStepState
{
};

class FMatchmakingAlgorithmStepError
    : public IMatchmakingAlgorithmStep<FMatchmakingAlgorithmStepStateError, FOnlineError>,
      public TSharedFromThis<FMatchmakingAlgorithmStepError>
{
public:
    virtual ~FMatchmakingAlgorithmStepError() override = default;
    UE_NONCOPYABLE(FMatchmakingAlgorithmStepError);
    FMatchmakingAlgorithmStepError() = default;

    virtual FName GetName() const override
    {
        return MMSTEP_ERROR;
    }

    virtual FText GetStatus() const override
    {
        return NSLOCTEXT("Matchmaking", "Error_Status", "Matchmaking failed");
    }

    virtual FText GetDetail() const override
    {
        return NSLOCTEXT("Matchmaking", "Error_Detail", "An error occurred while matchmaking.");
    }

    virtual TSharedPtr<FMatchmakingAlgorithmStepStateError> CreateNewState() override;
    virtual void Start(TSharedRef<FMatchmakingEngineTask> Task, FOnlineError Error) override;
};

#endif

REDPOINT_EOS_CODE_GUARD_END()