// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "Engine/World.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStep.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMSTEP_CONNECT_TO_GAME_SERVER FName("ConnectToGameServer")

class FMatchmakingAlgorithmStepStateConnectToGameServer : public IMatchmakingAlgorithmStepState
{
public:
    FDelegateHandle WorldInitHandle;
    bool bEventHandled;

    FMatchmakingAlgorithmStepStateConnectToGameServer();
};

class FMatchmakingAlgorithmStepConnectToGameServer
    : public IMatchmakingAlgorithmStep<FMatchmakingAlgorithmStepStateConnectToGameServer, FString>,
      public TSharedFromThis<FMatchmakingAlgorithmStepConnectToGameServer>
{
private:
    void OnMapStart(const UWorld::FActorsInitializedParams &ActorInitParams, TWeakPtr<FMatchmakingEngineTask> TaskWk);

public:
    virtual ~FMatchmakingAlgorithmStepConnectToGameServer() override = default;
    UE_NONCOPYABLE(FMatchmakingAlgorithmStepConnectToGameServer);
    FMatchmakingAlgorithmStepConnectToGameServer() = default;

    virtual FName GetName() const override
    {
        return MMSTEP_CONNECT_TO_GAME_SERVER;
    }

    virtual FText GetStatus() const override
    {
        return NSLOCTEXT("Matchmaking", "ConnectToGameServer_Status", "Joining the server");
    }

    virtual FText GetDetail() const override
    {
        return NSLOCTEXT("Matchmaking", "ConnectToGameServer_Detail", "Joining the game server...");
    }

    virtual TSharedPtr<FMatchmakingAlgorithmStepStateConnectToGameServer> CreateNewState() override;
    virtual void Start(TSharedRef<FMatchmakingEngineTask> Task, FString ConnectionUrl) override;
};

#endif

REDPOINT_EOS_CODE_GUARD_END()