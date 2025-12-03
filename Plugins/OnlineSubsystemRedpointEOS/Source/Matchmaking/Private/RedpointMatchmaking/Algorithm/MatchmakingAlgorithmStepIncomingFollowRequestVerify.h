// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStep.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMSTEP_INCOMING_FOLLOW_REQUEST_VERIFY FName("IncomingFollowRequestVerify")

class FMatchmakingAlgorithmStepStateIncomingFollowRequestVerify : public IMatchmakingAlgorithmStepState
{
};

class FMatchmakingAlgorithmStepIncomingFollowRequestVerify
    : public IMatchmakingAlgorithmStep<
          FMatchmakingAlgorithmStepStateIncomingFollowRequestVerify,
          TSharedPtr<FOnlineLobbyId>,
          TSharedRef<const FUniqueNetId>,
          int32>,
      public TSharedFromThis<FMatchmakingAlgorithmStepIncomingFollowRequestVerify>
{
private:
    void OnConnected(
        const FOnlineError &Error,
        const FUniqueNetId &UserId,
        const TSharedPtr<class FOnlineLobby> &Lobby,
        TWeakPtr<FMatchmakingEngineTask> TaskWk,
        TSharedPtr<FOnlineLobbyId> TargetLobbyId,
        TSharedRef<const FUniqueNetId> TargetMemberId,
        int32 TargetRequestId);

public:
    virtual ~FMatchmakingAlgorithmStepIncomingFollowRequestVerify() override = default;
    UE_NONCOPYABLE(FMatchmakingAlgorithmStepIncomingFollowRequestVerify);
    FMatchmakingAlgorithmStepIncomingFollowRequestVerify() = default;

    virtual FName GetName() const override
    {
        return MMSTEP_INCOMING_FOLLOW_REQUEST_VERIFY;
    }

    virtual FText GetStatus() const override
    {
        return NSLOCTEXT("Matchmaking", "IncomingFollowRequestVerify_Status", "Checking incoming request");
    }

    virtual FText GetDetail() const override
    {
        return NSLOCTEXT("Matchmaking", "IncomingFollowRequestVerify_Detail", "Connecting to remote lobby...");
    }

    virtual TSharedPtr<FMatchmakingAlgorithmStepStateIncomingFollowRequestVerify> CreateNewState() override;
    virtual void Start(
        TSharedRef<FMatchmakingEngineTask> Task,
        TSharedPtr<FOnlineLobbyId> TargetLobbyId,
        TSharedRef<const FUniqueNetId> TargetMemberId,
        int32 TargetRequestId) override;
};

#endif

REDPOINT_EOS_CODE_GUARD_END()