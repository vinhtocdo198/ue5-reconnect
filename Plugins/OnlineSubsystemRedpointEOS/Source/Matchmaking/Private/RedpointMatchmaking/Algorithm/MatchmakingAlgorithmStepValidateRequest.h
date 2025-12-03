// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStep.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMSTEP_VALIDATE_REQUEST FName("ValidateRequest")

class FMatchmakingAlgorithmStepStateValidateRequest : public IMatchmakingAlgorithmStepState
{
};

class FMatchmakingAlgorithmStepValidateRequest
    : public IMatchmakingAlgorithmStep<FMatchmakingAlgorithmStepStateValidateRequest>,
      public TSharedFromThis<FMatchmakingAlgorithmStepValidateRequest>
{
public:
    virtual ~FMatchmakingAlgorithmStepValidateRequest() override = default;
    UE_NONCOPYABLE(FMatchmakingAlgorithmStepValidateRequest);
    FMatchmakingAlgorithmStepValidateRequest() = default;

    virtual FName GetName() const override
    {
        return MMSTEP_VALIDATE_REQUEST;
    }

    virtual FText GetStatus() const override
    {
        return NSLOCTEXT("Matchmaking", "ValidateRequest_Status", "Starting matchmaking");
    }

    virtual FText GetDetail() const override
    {
        return NSLOCTEXT("Matchmaking", "ValidateRequest_Detail", "Validating request...");
    }

    virtual TSharedPtr<FMatchmakingAlgorithmStepStateValidateRequest> CreateNewState() override;
    virtual void Start(TSharedRef<FMatchmakingEngineTask> Task) override;
};

#endif

REDPOINT_EOS_CODE_GUARD_END()