// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStep.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMSTEP_CHAIN_FOLLOW_ONTO_NEW_HOST FName("ChainFollowOntoNewHost")

class FMatchmakingAlgorithmStepStateChainFollowOntoNewHost : public IMatchmakingAlgorithmStepState
{
};

class FMatchmakingAlgorithmStepChainFollowOntoNewHost
    : public IMatchmakingAlgorithmStep<FMatchmakingAlgorithmStepStateChainFollowOntoNewHost>,
      public TSharedFromThis<FMatchmakingAlgorithmStepChainFollowOntoNewHost>
{
private:
    void OnOldLobbyDisconnected(
        const FOnlineError &Error,
        const FUniqueNetId &UserId,
        TWeakPtr<FMatchmakingEngineTask> TaskWk,
        TSharedPtr<const FOnlineLobbyId> NewLobbyId,
        TSharedPtr<const FOnlineLobbyId> OldLobbyId);

    void OnNewLobbyConnected(
        const FOnlineError &Error,
        const FUniqueNetId &UserId,
        const TSharedPtr<class FOnlineLobby> &Lobby,
        TWeakPtr<FMatchmakingEngineTask> TaskWk,
        TSharedPtr<const FOnlineLobbyId> NewLobbyId,
        int AttemptNum);

public:
    virtual ~FMatchmakingAlgorithmStepChainFollowOntoNewHost() override = default;
    UE_NONCOPYABLE(FMatchmakingAlgorithmStepChainFollowOntoNewHost);
    FMatchmakingAlgorithmStepChainFollowOntoNewHost() = default;

    virtual FName GetName() const override
    {
        return MMSTEP_CHAIN_FOLLOW_ONTO_NEW_HOST;
    }

    virtual FText GetStatus() const override
    {
        return NSLOCTEXT("Matchmaking", "ChainFollowOntoNewHost_Status", "Waiting for players");
    }

    virtual FText GetDetail() const override
    {
        return NSLOCTEXT(
            "Matchmaking",
            "ChainFollowOntoNewHost_Detail",
            "Waiting for the host to find more players...");
    }

    virtual TSharedPtr<FMatchmakingAlgorithmStepStateChainFollowOntoNewHost> CreateNewState() override;
    virtual void Start(TSharedRef<FMatchmakingEngineTask> Task) override;
};

#endif

REDPOINT_EOS_CODE_GUARD_END()