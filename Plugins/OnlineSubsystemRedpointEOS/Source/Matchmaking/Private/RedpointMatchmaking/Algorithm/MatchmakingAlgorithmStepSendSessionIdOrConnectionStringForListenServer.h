// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStep.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMSTEP_SEND_SESSION_ID_OR_CONNECTION_STRING_FOR_LISTEN_SERVER                                                  \
    FName("SendSessionIdOrConnectionStringForListenServer")

class FMatchmakingAlgorithmStepStateSendSessionIdOrConnectionStringForListenServer
    : public IMatchmakingAlgorithmStepState
{
public:
    bool bHasFinishedLobbyUpdate;
    FDelegateHandle OnTimeoutHandle;
};

class FMatchmakingAlgorithmStepSendSessionIdOrConnectionStringForListenServer
    : public IMatchmakingAlgorithmStep<FMatchmakingAlgorithmStepStateSendSessionIdOrConnectionStringForListenServer>,
      public TSharedFromThis<FMatchmakingAlgorithmStepSendSessionIdOrConnectionStringForListenServer>
{
private:
    void CheckLobbyConnecting(const TSharedRef<FMatchmakingEngineTask> &Task);

    void OnTimeout(TWeakPtr<FMatchmakingEngineTask> TaskWk);

    void OnLobbyUpdated(const FOnlineError &Error, const FUniqueNetId &UserId, TWeakPtr<FMatchmakingEngineTask> TaskWk);

public:
    virtual ~FMatchmakingAlgorithmStepSendSessionIdOrConnectionStringForListenServer() override = default;
    UE_NONCOPYABLE(FMatchmakingAlgorithmStepSendSessionIdOrConnectionStringForListenServer);
    FMatchmakingAlgorithmStepSendSessionIdOrConnectionStringForListenServer() = default;

    virtual FName GetName() const override
    {
        return MMSTEP_SEND_SESSION_ID_OR_CONNECTION_STRING_FOR_LISTEN_SERVER;
    }

    virtual FText GetStatus() const override
    {
        return NSLOCTEXT("Matchmaking", "SendSessionIdOrConnectionStringForListenServer_Status", "Starting the server");
    }

    virtual FText GetDetail() const override
    {
        return NSLOCTEXT(
            "Matchmaking",
            "SendSessionIdOrConnectionStringForListenServer_Detail",
            "Starting the game server...");
    }

    virtual TSharedPtr<FMatchmakingAlgorithmStepStateSendSessionIdOrConnectionStringForListenServer> CreateNewState()
        override;
    virtual void Start(TSharedRef<FMatchmakingEngineTask> Task) override;
};

#endif

REDPOINT_EOS_CODE_GUARD_END()