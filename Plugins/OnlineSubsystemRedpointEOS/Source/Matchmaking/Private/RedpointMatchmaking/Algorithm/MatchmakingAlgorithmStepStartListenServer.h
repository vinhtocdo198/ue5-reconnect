// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "Engine/World.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStep.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMSTEP_START_LISTEN_SERVER FName("StartListenServer")

class FMatchmakingAlgorithmStepStateStartListenServer : public IMatchmakingAlgorithmStepState
{
public:
    FDelegateHandle WorldInitHandle;
};

class FMatchmakingAlgorithmStepStartListenServer
    : public IMatchmakingAlgorithmStep<FMatchmakingAlgorithmStepStateStartListenServer>,
      public TSharedFromThis<FMatchmakingAlgorithmStepStartListenServer>
{
private:
    void OnMapListening(
        const UWorld::FActorsInitializedParams &ActorInitParams,
        TWeakPtr<FMatchmakingEngineTask> TaskWk);

public:
    virtual ~FMatchmakingAlgorithmStepStartListenServer() override = default;
    UE_NONCOPYABLE(FMatchmakingAlgorithmStepStartListenServer);
    FMatchmakingAlgorithmStepStartListenServer() = default;

    virtual FName GetName() const override
    {
        return MMSTEP_START_LISTEN_SERVER;
    }

    virtual FText GetStatus() const override
    {
        return NSLOCTEXT("Matchmaking", "StartListenServer_Status", "Starting the server");
    }

    virtual FText GetDetail() const override
    {
        return NSLOCTEXT("Matchmaking", "StartListenServer_Detail", "Starting the game server...");
    }

    virtual TSharedPtr<FMatchmakingAlgorithmStepStateStartListenServer> CreateNewState() override;
    virtual void Start(TSharedRef<FMatchmakingEngineTask> Task) override;
};

#endif

REDPOINT_EOS_CODE_GUARD_END()