// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStep.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMSTEP_INCOMING_FOLLOW_REQUEST_REJECT FName("IncomingFollowRequestReject")

class FMatchmakingAlgorithmStepStateIncomingFollowRequestReject : public IMatchmakingAlgorithmStepState
{
};

class FMatchmakingAlgorithmStepIncomingFollowRequestReject
    : public IMatchmakingAlgorithmStep<
          FMatchmakingAlgorithmStepStateIncomingFollowRequestReject,
          TSharedPtr<FOnlineLobbyId>,
          TSharedRef<const FUniqueNetId>,
          int32>,
      public TSharedFromThis<FMatchmakingAlgorithmStepIncomingFollowRequestReject>
{
private:
    void OnFollowResponseSent(
        const FOnlineError &Error,
        const FUniqueNetId &UserId,
        TWeakPtr<FMatchmakingEngineTask> TaskWk);

public:
    virtual ~FMatchmakingAlgorithmStepIncomingFollowRequestReject() override = default;
    UE_NONCOPYABLE(FMatchmakingAlgorithmStepIncomingFollowRequestReject);
    FMatchmakingAlgorithmStepIncomingFollowRequestReject() = default;

    virtual FName GetName() const override
    {
        return MMSTEP_INCOMING_FOLLOW_REQUEST_REJECT;
    }

    virtual FText GetStatus() const override
    {
        return NSLOCTEXT("Matchmaking", "IncomingFollowRequestReject_Status", "Checking incoming request");
    }

    virtual FText GetDetail() const override
    {
        return NSLOCTEXT("Matchmaking", "IncomingFollowRequestReject_Detail", "Going back to searching...");
    }

    virtual TSharedPtr<FMatchmakingAlgorithmStepStateIncomingFollowRequestReject> CreateNewState() override;
    virtual void Start(
        TSharedRef<FMatchmakingEngineTask> Task,
        TSharedPtr<FOnlineLobbyId> TargetLobbyId,
        TSharedRef<const FUniqueNetId> TargetMemberId,
        int32 TargetRequestId) override;
};

#endif

REDPOINT_EOS_CODE_GUARD_END()