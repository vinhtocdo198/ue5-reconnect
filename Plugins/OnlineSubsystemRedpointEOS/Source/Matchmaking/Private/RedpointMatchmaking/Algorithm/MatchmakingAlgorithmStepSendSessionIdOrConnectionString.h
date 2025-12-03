// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStep.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMSTEP_SEND_SESSION_ID_OR_CONNECTION_STRING FName("SendSessionIdOrConnectionString")

class FMatchmakingAlgorithmStepStateSendSessionIdOrConnectionString : public IMatchmakingAlgorithmStepState
{
public:
    bool bHasFinishedLobbyUpdate;
    FDelegateHandle OnTimeoutHandle;
    FName NextStep;
    FString ConnectionKey;
    FString ConnectionValue;
};

class FMatchmakingAlgorithmStepSendSessionIdOrConnectionString
    : public IMatchmakingAlgorithmStep<
          FMatchmakingAlgorithmStepStateSendSessionIdOrConnectionString,
          FName,
          FString,
          FString>,
      public TSharedFromThis<FMatchmakingAlgorithmStepSendSessionIdOrConnectionString>
{
private:
    void CheckLobbyConnecting(const TSharedRef<FMatchmakingEngineTask> &Task);

    void OnTimeout(TWeakPtr<FMatchmakingEngineTask> TaskWk);

    void OnLobbyUpdated(const FOnlineError &Error, const FUniqueNetId &UserId, TWeakPtr<FMatchmakingEngineTask> TaskWk);

public:
    virtual ~FMatchmakingAlgorithmStepSendSessionIdOrConnectionString() override = default;
    UE_NONCOPYABLE(FMatchmakingAlgorithmStepSendSessionIdOrConnectionString);
    FMatchmakingAlgorithmStepSendSessionIdOrConnectionString() = default;

    virtual FName GetName() const override
    {
        return MMSTEP_SEND_SESSION_ID_OR_CONNECTION_STRING;
    }

    virtual FText GetStatus() const override
    {
        return NSLOCTEXT("Matchmaking", "SendSessionIdOrConnectionString_Status", "Notifying other players");
    }

    virtual FText GetDetail() const override
    {
        return NSLOCTEXT(
            "Matchmaking",
            "SendSessionIdOrConnectionString_Detail",
            "Sending connection information to other players...");
    }

    virtual TSharedPtr<FMatchmakingAlgorithmStepStateSendSessionIdOrConnectionString> CreateNewState() override;
    virtual void Start(
        TSharedRef<FMatchmakingEngineTask> Task,
        FName NextStep,
        FString ConnectionKey,
        FString ConnectionValue) override;

    virtual bool CanHandleInterrupt(const TSharedRef<FMatchmakingEngineTask> &Task, FName InterruptType) override;
    virtual bool HandleInterrupt(const TSharedRef<FMatchmakingEngineTask> &Task, FName InterruptType) override;

    void StartLobbyUpdate(TSharedRef<FMatchmakingEngineTask> Task);
};

#endif

REDPOINT_EOS_CODE_GUARD_END()