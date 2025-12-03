// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStep.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMSTEP_WAITING_FOR_PLAYERS_TO_JOIN FName("WaitingForPlayersToJoin")

class FMatchmakingAlgorithmStepStateWaitingForPlayersToJoin : public IMatchmakingAlgorithmStepState
{
public:
    FDelegateHandle OnTimeoutHandle;
    FDelegateHandle OnPartyDataReceivedHandle;
};

class FMatchmakingAlgorithmStepWaitingForPlayersToJoin
    : public IMatchmakingAlgorithmStep<FMatchmakingAlgorithmStepStateWaitingForPlayersToJoin>,
      public TSharedFromThis<FMatchmakingAlgorithmStepWaitingForPlayersToJoin>
{
private:
    void OnTimeout(TWeakPtr<FMatchmakingEngineTask> TaskWk);
    void OnPartyDataUpdated(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const FName &Namespace,
        const FOnlinePartyData &PartyData,
        TWeakPtr<FMatchmakingEngineTask> TaskWk);

public:
    virtual ~FMatchmakingAlgorithmStepWaitingForPlayersToJoin() override = default;
    UE_NONCOPYABLE(FMatchmakingAlgorithmStepWaitingForPlayersToJoin);
    FMatchmakingAlgorithmStepWaitingForPlayersToJoin() = default;

    virtual FName GetName() const override
    {
        return MMSTEP_WAITING_FOR_PLAYERS_TO_JOIN;
    }

    virtual FText GetStatus() const override
    {
        return NSLOCTEXT("Matchmaking", "WaitingForPlayersToJoin_Status", "Waiting for players");
    }

    virtual FText GetDetail() const override
    {
        return NSLOCTEXT("Matchmaking", "WaitingForPlayersToJoin_Detail", "Waiting for other players to join...");
    }

    virtual TSharedPtr<FMatchmakingAlgorithmStepStateWaitingForPlayersToJoin> CreateNewState() override;
    virtual void Start(TSharedRef<FMatchmakingEngineTask> Task) override;
    virtual bool CanHandleInterrupt(const TSharedRef<FMatchmakingEngineTask> &Task, FName InterruptType) override;
    virtual bool HandleInterrupt(const TSharedRef<FMatchmakingEngineTask> &Task, FName InterruptType) override;
};

#endif

REDPOINT_EOS_CODE_GUARD_END()