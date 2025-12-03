// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStep.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMSTEP_COMPLETE_MATCH_FIRE_EVENT FName("CompleteMatchFireEvent")

class FMatchmakingAlgorithmStepStateCompleteMatchFireEvent : public IMatchmakingAlgorithmStepState
{
};

class FMatchmakingAlgorithmStepCompleteMatchFireEvent
    : public IMatchmakingAlgorithmStep<FMatchmakingAlgorithmStepStateCompleteMatchFireEvent>,
      public TSharedFromThis<FMatchmakingAlgorithmStepCompleteMatchFireEvent>
{
public:
    FMatchmakingAlgorithmStepCompleteMatchFireEvent() = default;
    virtual ~FMatchmakingAlgorithmStepCompleteMatchFireEvent() override = default;
    UE_NONCOPYABLE(FMatchmakingAlgorithmStepCompleteMatchFireEvent);

    virtual FName GetName() const override
    {
        return MMSTEP_COMPLETE_MATCH_FIRE_EVENT;
    }

    virtual FText GetStatus() const override
    {
        return NSLOCTEXT("Matchmaking", "CompleteMatchFireEvent_Status", "Finishing up");
    }

    virtual FText GetDetail() const override
    {
        return NSLOCTEXT("Matchmaking", "CompleteMatchFireEvent_Detail", "Preparing to finish matchmaking...");
    }

    virtual TSharedPtr<FMatchmakingAlgorithmStepStateCompleteMatchFireEvent> CreateNewState() override;
    virtual void Start(TSharedRef<FMatchmakingEngineTask> Task) override;
};

#endif

REDPOINT_EOS_CODE_GUARD_END()