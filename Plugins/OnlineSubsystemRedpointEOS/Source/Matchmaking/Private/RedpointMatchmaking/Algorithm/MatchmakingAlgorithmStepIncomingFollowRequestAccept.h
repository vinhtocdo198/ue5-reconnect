// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStep.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepFollowHost.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMSTEP_INCOMING_FOLLOW_REQUEST_ACCEPT FName("IncomingFollowRequestAccept")

class FMatchmakingAlgorithmStepStateIncomingFollowRequestAccept : public IMatchmakingAlgorithmStepState
{
};

class FMatchmakingAlgorithmStepIncomingFollowRequestAccept
    : public IMatchmakingAlgorithmStep<
          FMatchmakingAlgorithmStepStateIncomingFollowRequestAccept,
          TSharedPtr<const FOnlineLobbyId>,
          TSharedRef<const FUniqueNetId>,
          int32>,
      public TSharedFromThis<FMatchmakingAlgorithmStepIncomingFollowRequestAccept>
{
private:
    void OnDeletedFollowRequestLobby(
        const FOnlineError &Error,
        const FUniqueNetId &UserId,
        TWeakPtr<FMatchmakingEngineTask> TaskWk,
        TSharedPtr<const FOnlineLobbyId> TargetLobbyId,
        TSharedRef<const FUniqueNetId> TargetMemberId);

    void OnDisconnectedFromMatchmakingLobby(
        const FOnlineError &Error,
        const FUniqueNetId &UserId,
        TWeakPtr<FMatchmakingEngineTask> TaskWk,
        TSharedPtr<const FOnlineLobbyId> TargetLobbyId,
        TSharedRef<const FUniqueNetId> TargetMemberId);

    void OnMatchmakingLobbyUpdated(
        const FOnlineError &Error,
        const FUniqueNetId &UserId,
        TWeakPtr<FMatchmakingEngineTask> TaskWk,
        TSharedPtr<const FOnlineLobbyId> TargetLobbyId,
        TSharedRef<const FUniqueNetId> TargetMemberId);

    void OnFollowResponseSent(
        const FOnlineError &Error,
        const FUniqueNetId &UserId,
        TWeakPtr<FMatchmakingEngineTask> TaskWk,
        TSharedPtr<const FOnlineLobbyId> TargetLobbyId,
        TSharedRef<const FUniqueNetId> TargetMemberId);

public:
    virtual ~FMatchmakingAlgorithmStepIncomingFollowRequestAccept() override = default;
    UE_NONCOPYABLE(FMatchmakingAlgorithmStepIncomingFollowRequestAccept);
    FMatchmakingAlgorithmStepIncomingFollowRequestAccept() = default;

    virtual FName GetName() const override
    {
        return MMSTEP_INCOMING_FOLLOW_REQUEST_ACCEPT;
    }

    virtual FText GetStatus() const override
    {
        return NSLOCTEXT("Matchmaking", "IncomingFollowRequestAccept_Status", "Checking incoming request");
    }

    virtual FText GetDetail() const override
    {
        return NSLOCTEXT("Matchmaking", "IncomingFollowRequestAccept_Detail", "Accepting request...");
    }

    virtual TSharedPtr<FMatchmakingAlgorithmStepStateIncomingFollowRequestAccept> CreateNewState() override;
    virtual void Start(
        TSharedRef<FMatchmakingEngineTask> Task,
        TSharedPtr<const FOnlineLobbyId> TargetLobbyId,
        TSharedRef<const FUniqueNetId> TargetMemberId,
        int32 TargetRequestId) override;
};

#endif

REDPOINT_EOS_CODE_GUARD_END()