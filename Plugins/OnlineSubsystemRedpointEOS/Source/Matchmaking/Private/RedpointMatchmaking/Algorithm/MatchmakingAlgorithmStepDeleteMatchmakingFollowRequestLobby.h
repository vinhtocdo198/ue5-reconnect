// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStep.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMSTEP_DELETE_MATCHMAKING_FOLLOW_REQUEST_LOBBY FName("DeleteMatchmakingFollowRequestLobby")

class FMatchmakingAlgorithmStepStateDeleteMatchmakingFollowRequestLobby : public IMatchmakingAlgorithmStepState
{
};

class FMatchmakingAlgorithmStepDeleteMatchmakingFollowRequestLobby
    : public IMatchmakingAlgorithmStep<FMatchmakingAlgorithmStepStateDeleteMatchmakingFollowRequestLobby, FName>,
      public TSharedFromThis<FMatchmakingAlgorithmStepDeleteMatchmakingFollowRequestLobby>
{
private:
    void OnLobbyDeleted(
        const FOnlineError &Error,
        const FUniqueNetId &UserId,
        TWeakPtr<FMatchmakingEngineTask> TaskWk,
        FName NextStep);

public:
    virtual ~FMatchmakingAlgorithmStepDeleteMatchmakingFollowRequestLobby() override = default;
    UE_NONCOPYABLE(FMatchmakingAlgorithmStepDeleteMatchmakingFollowRequestLobby);
    FMatchmakingAlgorithmStepDeleteMatchmakingFollowRequestLobby() = default;

    virtual FName GetName() const override
    {
        return MMSTEP_DELETE_MATCHMAKING_FOLLOW_REQUEST_LOBBY;
    }

    virtual FText GetStatus() const override
    {
        return NSLOCTEXT("Matchmaking", "DeleteMatchmakingFollowRequestLobby_Status", "Finishing up");
    }

    virtual FText GetDetail() const override
    {
        return NSLOCTEXT(
            "Matchmaking",
            "DeleteMatchmakingFollowRequestLobby_Detail",
            "Closing follow request lobby...");
    }

    virtual TSharedPtr<FMatchmakingAlgorithmStepStateDeleteMatchmakingFollowRequestLobby> CreateNewState() override;
    virtual void Start(TSharedRef<FMatchmakingEngineTask> Task, FName NextStep) override;
};

#endif

REDPOINT_EOS_CODE_GUARD_END()