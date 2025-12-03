// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStep.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMSTEP_FIND_DEDICATED_SERVER_SESSION FName("FindDedicatedServerSession")

class FMatchmakingAlgorithmStepStateFindDedicatedServerSession : public IMatchmakingAlgorithmStepState
{
public:
    FDelegateHandle FindSessionsDelegateHandle;
    FTSTicker::FDelegateHandle ScheduledRetryHandle;
};

class FMatchmakingAlgorithmStepFindDedicatedServerSession
    : public IMatchmakingAlgorithmStep<FMatchmakingAlgorithmStepStateFindDedicatedServerSession>,
      public TSharedFromThis<FMatchmakingAlgorithmStepFindDedicatedServerSession>
{
private:
    void OnFindSessionsComplete(
        bool bWasSuccessful,
        TWeakPtr<FMatchmakingEngineTask> TaskWk,
        TSharedRef<FOnlineSessionSearch> Search,
        int32 AttemptNumber);
    bool SearchForDedicatedServerSession(
        float DeltaSeconds,
        TWeakPtr<FMatchmakingEngineTask> TaskWk,
        int32 AttemptNumber);

public:
    virtual ~FMatchmakingAlgorithmStepFindDedicatedServerSession() override = default;
    UE_NONCOPYABLE(FMatchmakingAlgorithmStepFindDedicatedServerSession);
    FMatchmakingAlgorithmStepFindDedicatedServerSession() = default;

    virtual FName GetName() const override
    {
        return MMSTEP_FIND_DEDICATED_SERVER_SESSION;
    }

    virtual FText GetStatus() const override
    {
        return NSLOCTEXT("Matchmaking", "FindDedicatedServerSession_Status", "Finding a server");
    }

    virtual FText GetDetail() const override
    {
        return NSLOCTEXT("Matchmaking", "FindDedicatedServerSession_Detail", "Finding a game server to join...");
    }

    virtual TSharedPtr<FMatchmakingAlgorithmStepStateFindDedicatedServerSession> CreateNewState() override;
    virtual void Start(TSharedRef<FMatchmakingEngineTask> Task) override;

    virtual bool CanHandleInterrupt(const TSharedRef<FMatchmakingEngineTask> &Task, FName InterruptType) override;
    virtual bool HandleInterrupt(const TSharedRef<FMatchmakingEngineTask> &Task, FName InterruptType) override;
};

#endif

REDPOINT_EOS_CODE_GUARD_END()