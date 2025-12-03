// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStep.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMSTEP_WAIT_FOR_PARTY_LEADER FName("WaitForPartyLeader")

class FMatchmakingAlgorithmStepStateWaitForPartyLeader : public IMatchmakingAlgorithmStepState
{
public:
    FDelegateHandle OnPartyDataReceivedHandle;
    FDelegateHandle OnPartyExitedHandle;
};

class FMatchmakingAlgorithmStepWaitForPartyLeader
    : public IMatchmakingAlgorithmStep<FMatchmakingAlgorithmStepStateWaitForPartyLeader>,
      public TSharedFromThis<FMatchmakingAlgorithmStepWaitForPartyLeader>
{
private:
    void OnConnected(
        const FOnlineError &Error,
        const FUniqueNetId &UserId,
        const TSharedPtr<class FOnlineLobby> &Lobby,
        TWeakPtr<FMatchmakingEngineTask> TaskWk,
        TSharedPtr<FOnlineLobbyId> TargetLobbyId);

    void OnPartyDataUpdated(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const FName &Namespace,
        const FOnlinePartyData &PartyData,
        TWeakPtr<FMatchmakingEngineTask> TaskWk);

    void OnPartyExited(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        TWeakPtr<FMatchmakingEngineTask> TaskWk);

public:
    virtual ~FMatchmakingAlgorithmStepWaitForPartyLeader() override = default;
    UE_NONCOPYABLE(FMatchmakingAlgorithmStepWaitForPartyLeader);
    FMatchmakingAlgorithmStepWaitForPartyLeader() = default;

    virtual FName GetName() const override
    {
        return MMSTEP_WAIT_FOR_PARTY_LEADER;
    }

    virtual FText GetStatus() const override
    {
        return NSLOCTEXT("Matchmaking", "NotifyExistingParty_Status", "Waiting");
    }

    virtual FText GetDetail() const override
    {
        return NSLOCTEXT("Matchmaking", "NotifyExistingParty_Detail", "Waiting for party leader...");
    }

    virtual TSharedPtr<FMatchmakingAlgorithmStepStateWaitForPartyLeader> CreateNewState() override;
    virtual void Start(TSharedRef<FMatchmakingEngineTask> Task) override;
};

#endif

REDPOINT_EOS_CODE_GUARD_END()