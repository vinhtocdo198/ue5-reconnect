// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStep.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMSTEP_COMPLETE_MATCH_AS_CLIENT FName("CompleteMatchAsClient")

class FMatchmakingAlgorithmStepStateCompleteMatchAsClient : public IMatchmakingAlgorithmStepState
{
};

class FMatchmakingAlgorithmStepCompleteMatchAsClient
    : public IMatchmakingAlgorithmStep<FMatchmakingAlgorithmStepStateCompleteMatchAsClient>,
      public TSharedFromThis<FMatchmakingAlgorithmStepCompleteMatchAsClient>
{
private:
    void OnMarkedAsFinalized(
        const FOnlineError &Error,
        const FUniqueNetId &UserId,
        TWeakPtr<FMatchmakingEngineTask> TaskWk,
        TSharedRef<FMatchmakingEngineResponse> Result);

public:
    virtual ~FMatchmakingAlgorithmStepCompleteMatchAsClient() override = default;
    UE_NONCOPYABLE(FMatchmakingAlgorithmStepCompleteMatchAsClient);
    FMatchmakingAlgorithmStepCompleteMatchAsClient() = default;

    virtual FName GetName() const override
    {
        return MMSTEP_COMPLETE_MATCH_AS_CLIENT;
    }

    virtual FText GetStatus() const override
    {
        return NSLOCTEXT("Matchmaking", "CompleteMatchAsClient_Status", "Finishing up");
    }

    virtual FText GetDetail() const override
    {
        return NSLOCTEXT("Matchmaking", "CompleteMatchAsClient_Detail", "Preparing to finish matchmaking...");
    }

    virtual TSharedPtr<FMatchmakingAlgorithmStepStateCompleteMatchAsClient> CreateNewState() override;
    virtual void Start(TSharedRef<FMatchmakingEngineTask> Task) override;
};

#endif

REDPOINT_EOS_CODE_GUARD_END()