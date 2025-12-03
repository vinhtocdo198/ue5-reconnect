// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStep.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMSTEP_CREATE_MATCHMAKING_FOLLOW_REQUEST_LOBBY FName("CreateMatchmakingFollowRequestLobby")

class FMatchmakingAlgorithmStepStateCreateMatchmakingFollowRequestLobby : public IMatchmakingAlgorithmStepState
{
};

class FMatchmakingAlgorithmStepCreateMatchmakingFollowRequestLobby
    : public IMatchmakingAlgorithmStep<FMatchmakingAlgorithmStepStateCreateMatchmakingFollowRequestLobby>,
      public TSharedFromThis<FMatchmakingAlgorithmStepCreateMatchmakingFollowRequestLobby>
{
private:
    void OnLobbyCreated(
        const FOnlineError &Error,
        const FUniqueNetId &UserId,
        const TSharedPtr<class FOnlineLobby> &Lobby,
        TWeakPtr<FMatchmakingEngineTask> TaskWk);

public:
    virtual ~FMatchmakingAlgorithmStepCreateMatchmakingFollowRequestLobby() override = default;
    UE_NONCOPYABLE(FMatchmakingAlgorithmStepCreateMatchmakingFollowRequestLobby);
    FMatchmakingAlgorithmStepCreateMatchmakingFollowRequestLobby() = default;

    virtual FName GetName() const override
    {
        return MMSTEP_CREATE_MATCHMAKING_FOLLOW_REQUEST_LOBBY;
    }

    virtual FText GetStatus() const override
    {
        return NSLOCTEXT("Matchmaking", "CreateMatchmakingFollowRequestLobby_Status", "Starting matchmaking");
    }

    virtual FText GetDetail() const override
    {
        return NSLOCTEXT(
            "Matchmaking",
            "CreateMatchmakingFollowRequestLobby_Detail",
            "Creating follow request lobby...");
    }

    virtual TSharedPtr<FMatchmakingAlgorithmStepStateCreateMatchmakingFollowRequestLobby> CreateNewState() override;
    virtual void Start(TSharedRef<FMatchmakingEngineTask> Task) override;
};

#endif

REDPOINT_EOS_CODE_GUARD_END()