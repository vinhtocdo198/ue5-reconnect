// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStep.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMSTEP_CREATE_MATCHMAKING_LOBBY FName("CreateMatchmakingLobby")

class FMatchmakingAlgorithmStepStateCreateMatchmakingLobby : public IMatchmakingAlgorithmStepState
{
};

class FMatchmakingAlgorithmStepCreateMatchmakingLobby
    : public IMatchmakingAlgorithmStep<FMatchmakingAlgorithmStepStateCreateMatchmakingLobby>,
      public TSharedFromThis<FMatchmakingAlgorithmStepCreateMatchmakingLobby>
{
private:
    void OnLobbyCreated(
        const FOnlineError &Error,
        const FUniqueNetId &UserId,
        const TSharedPtr<class FOnlineLobby> &Lobby,
        TWeakPtr<FMatchmakingEngineTask> TaskWk);

public:
    virtual ~FMatchmakingAlgorithmStepCreateMatchmakingLobby() override = default;
    UE_NONCOPYABLE(FMatchmakingAlgorithmStepCreateMatchmakingLobby);
    FMatchmakingAlgorithmStepCreateMatchmakingLobby() = default;

    virtual FName GetName() const override
    {
        return MMSTEP_CREATE_MATCHMAKING_LOBBY;
    }

    virtual FText GetStatus() const override
    {
        return NSLOCTEXT("Matchmaking", "CreateMatchmakingLobby_Status", "Starting matchmaking");
    }

    virtual FText GetDetail() const override
    {
        return NSLOCTEXT("Matchmaking", "CreateMatchmakingLobby_Detail", "Creating matchmaking lobby...");
    }

    virtual TSharedPtr<FMatchmakingAlgorithmStepStateCreateMatchmakingLobby> CreateNewState() override;
    virtual void Start(TSharedRef<FMatchmakingEngineTask> Task) override;
};

#endif

REDPOINT_EOS_CODE_GUARD_END()