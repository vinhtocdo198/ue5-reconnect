// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStep.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMSTEP_DELETE_MATCHMAKING_LOBBY FName("DeleteMatchmakingLobby")

class FMatchmakingAlgorithmStepStateDeleteMatchmakingLobby : public IMatchmakingAlgorithmStepState
{
};

class FMatchmakingAlgorithmStepDeleteMatchmakingLobby
    : public IMatchmakingAlgorithmStep<FMatchmakingAlgorithmStepStateDeleteMatchmakingLobby, FName>,
      public TSharedFromThis<FMatchmakingAlgorithmStepDeleteMatchmakingLobby>
{
private:
    void OnLobbyDeleted(
        const FOnlineError &Error,
        const FUniqueNetId &UserId,
        TWeakPtr<FMatchmakingEngineTask> TaskWk,
        FName NextStep);

public:
    virtual ~FMatchmakingAlgorithmStepDeleteMatchmakingLobby() override = default;
    UE_NONCOPYABLE(FMatchmakingAlgorithmStepDeleteMatchmakingLobby);
    FMatchmakingAlgorithmStepDeleteMatchmakingLobby() = default;

    virtual FName GetName() const override
    {
        return MMSTEP_DELETE_MATCHMAKING_LOBBY;
    }

    virtual FText GetStatus() const override
    {
        return NSLOCTEXT("Matchmaking", "DeleteMatchmakingLobby_Status", "Finishing up");
    }

    virtual FText GetDetail() const override
    {
        return NSLOCTEXT("Matchmaking", "DeleteMatchmakingLobby_Detail", "Closing lobby...");
    }

    virtual TSharedPtr<FMatchmakingAlgorithmStepStateDeleteMatchmakingLobby> CreateNewState() override;
    virtual void Start(TSharedRef<FMatchmakingEngineTask> Task, FName NextStep) override;
};

#endif

REDPOINT_EOS_CODE_GUARD_END()