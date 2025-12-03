// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "Containers/Ticker.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStep.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMSTEP_FOLLOW_HOST FName("FollowHost")

class FMatchmakingAlgorithmStepStateFollowHost : public IMatchmakingAlgorithmStepState
{
public:
    FTSTicker::FDelegateHandle LostHeartbeatHandle;
    TOptional<FDateTime> LastHeartbeatTimestamp;
};

class FMatchmakingAlgorithmStepFollowHost : public IMatchmakingAlgorithmStep<FMatchmakingAlgorithmStepStateFollowHost>,
                                            public TSharedFromThis<FMatchmakingAlgorithmStepFollowHost>
{
private:
    bool OnHeartbeatLost(float DeltaSeconds, TWeakPtr<FMatchmakingEngineTask> TaskWk);

public:
    virtual ~FMatchmakingAlgorithmStepFollowHost() override = default;
    UE_NONCOPYABLE(FMatchmakingAlgorithmStepFollowHost);
    FMatchmakingAlgorithmStepFollowHost() = default;

    virtual FName GetName() const override
    {
        return MMSTEP_FOLLOW_HOST;
    }

    virtual FText GetStatus() const override
    {
        return NSLOCTEXT("Matchmaking", "FollowHost_Status", "Waiting for players");
    }

    virtual FText GetDetail() const override
    {
        return NSLOCTEXT("Matchmaking", "FollowHost_Detail", "Waiting for the host to find more players...");
    }

    virtual TSharedPtr<FMatchmakingAlgorithmStepStateFollowHost> CreateNewState() override;
    virtual void Start(TSharedRef<FMatchmakingEngineTask> Task) override;
    virtual bool CanHandleInterrupt(const TSharedRef<FMatchmakingEngineTask> &Task, FName InterruptType) override;
    virtual bool HandleInterrupt(const TSharedRef<FMatchmakingEngineTask> &Task, FName InterruptType) override;
};

#endif

REDPOINT_EOS_CODE_GUARD_END()