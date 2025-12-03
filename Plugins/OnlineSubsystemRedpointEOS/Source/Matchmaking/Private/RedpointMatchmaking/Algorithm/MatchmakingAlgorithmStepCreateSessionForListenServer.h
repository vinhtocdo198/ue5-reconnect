// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStep.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMSTEP_CREATE_SESSION_FOR_LISTEN_SERVER FName("CreateSessionForListenServer")

class FMatchmakingAlgorithmStepStateCreateSessionForListenServer : public IMatchmakingAlgorithmStepState
{
public:
    FDelegateHandle SessionCreateHandle;
    FDelegateHandle SessionDestroyHandle;
};

class FMatchmakingAlgorithmStepCreateSessionForListenServer
    : public IMatchmakingAlgorithmStep<FMatchmakingAlgorithmStepStateCreateSessionForListenServer>,
      public TSharedFromThis<FMatchmakingAlgorithmStepCreateSessionForListenServer>
{
private:
    void OnSessionCreated(FName SessionName, bool bWasSuccessful, TWeakPtr<FMatchmakingEngineTask> TaskWk);

    void CreateSession(TSharedRef<FMatchmakingEngineTask> Task);

    void OnSessionDestroyed(FName SessionName, bool bWasSuccessful, TWeakPtr<FMatchmakingEngineTask> TaskWk);

public:
    virtual ~FMatchmakingAlgorithmStepCreateSessionForListenServer() override = default;
    UE_NONCOPYABLE(FMatchmakingAlgorithmStepCreateSessionForListenServer);
    FMatchmakingAlgorithmStepCreateSessionForListenServer() = default;

    virtual FName GetName() const override
    {
        return MMSTEP_CREATE_SESSION_FOR_LISTEN_SERVER;
    }

    virtual FText GetStatus() const override
    {
        return NSLOCTEXT("Matchmaking", "CreateSessionForListenServer_Status", "Starting the server");
    }

    virtual FText GetDetail() const override
    {
        return NSLOCTEXT("Matchmaking", "CreateSessionForListenServer_Detail", "Starting the game server...");
    }

    virtual TSharedPtr<FMatchmakingAlgorithmStepStateCreateSessionForListenServer> CreateNewState() override;
    virtual void Start(TSharedRef<FMatchmakingEngineTask> Task) override;
};

#endif

REDPOINT_EOS_CODE_GUARD_END()