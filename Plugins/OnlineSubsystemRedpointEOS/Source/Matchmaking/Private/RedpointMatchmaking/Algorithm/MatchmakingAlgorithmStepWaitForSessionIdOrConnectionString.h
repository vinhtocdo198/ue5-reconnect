// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStep.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMSTEP_WAIT_FOR_SESSION_ID_OR_CONNECTION_STRING FName("WaitForSessionIdOrConnectionString")

class FMatchmakingAlgorithmStepStateWaitForSessionIdOrConnectionString : public IMatchmakingAlgorithmStepState
{
public:
    bool bIsMarkingAsConnecting;
};

class FMatchmakingAlgorithmStepWaitForSessionIdOrConnectionString
    : public IMatchmakingAlgorithmStep<FMatchmakingAlgorithmStepStateWaitForSessionIdOrConnectionString>,
      public TSharedFromThis<FMatchmakingAlgorithmStepWaitForSessionIdOrConnectionString>
{
public:
    virtual ~FMatchmakingAlgorithmStepWaitForSessionIdOrConnectionString() override = default;
    UE_NONCOPYABLE(FMatchmakingAlgorithmStepWaitForSessionIdOrConnectionString);
    FMatchmakingAlgorithmStepWaitForSessionIdOrConnectionString() = default;

    virtual FName GetName() const override
    {
        return MMSTEP_WAIT_FOR_SESSION_ID_OR_CONNECTION_STRING;
    }

    virtual FText GetStatus() const override
    {
        return NSLOCTEXT("Matchmaking", "WaitForSessionIdOrConnectionString_Status", "Waiting for host");
    }

    virtual FText GetDetail() const override
    {
        return NSLOCTEXT(
            "Matchmaking",
            "WaitForSessionIdOrConnectionString_Detail",
            "Waiting for the game to start...");
    }

    virtual TSharedPtr<FMatchmakingAlgorithmStepStateWaitForSessionIdOrConnectionString> CreateNewState() override;
    virtual void Start(TSharedRef<FMatchmakingEngineTask> Task) override;

    virtual bool CanHandleInterrupt(const TSharedRef<FMatchmakingEngineTask> &Task, FName InterruptType) override;
    virtual bool HandleInterrupt(const TSharedRef<FMatchmakingEngineTask> &Task, FName InterruptType) override;

private:
    void OnMarkedAsConnecting(
        const FOnlineError &Error,
        const FUniqueNetId &UserId,
        TWeakPtr<FMatchmakingEngineTask> TaskWk,
        FName NextStep,
        FString ConnectionUrlOrSessionId);
};

#endif

REDPOINT_EOS_CODE_GUARD_END()