// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStep.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMSTEP_JOIN_SERVER_SESSION FName("JoinServerSession")

class FMatchmakingAlgorithmStepStateJoinServerSession : public IMatchmakingAlgorithmStepState
{
};

class FMatchmakingAlgorithmStepJoinServerSession
    : public IMatchmakingAlgorithmStep<FMatchmakingAlgorithmStepStateJoinServerSession, FString>,
      public TSharedFromThis<FMatchmakingAlgorithmStepJoinServerSession>
{
private:
    void OnSessionFindComplete(
        int32 LocalUserNum,
        bool bWasSuccessful,
        const FOnlineSessionSearchResult &SearchResult,
        TWeakPtr<FMatchmakingEngineTask> TaskWk,
        FString SessionId,
        int32 AttemptNumber);

public:
    virtual ~FMatchmakingAlgorithmStepJoinServerSession() override = default;
    UE_NONCOPYABLE(FMatchmakingAlgorithmStepJoinServerSession);
    FMatchmakingAlgorithmStepJoinServerSession() = default;

    virtual FName GetName() const override
    {
        return MMSTEP_JOIN_SERVER_SESSION;
    }

    virtual FText GetStatus() const override
    {
        return NSLOCTEXT("Matchmaking", "JoinServerSession_Status", "Joining the server");
    }

    virtual FText GetDetail() const override
    {
        return NSLOCTEXT("Matchmaking", "JoinServerSession_Detail", "Joining the game server...");
    }

    virtual TSharedPtr<FMatchmakingAlgorithmStepStateJoinServerSession> CreateNewState() override;
    virtual void Start(TSharedRef<FMatchmakingEngineTask> Task, FString SessionId) override;
};

#endif

REDPOINT_EOS_CODE_GUARD_END()