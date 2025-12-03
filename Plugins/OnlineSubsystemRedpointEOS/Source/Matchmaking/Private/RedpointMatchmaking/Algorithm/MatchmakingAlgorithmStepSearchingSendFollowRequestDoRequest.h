// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "../MatchmakingCandidateEvaluator.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStep.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMSTEP_SEARCHING_SEND_FOLLOW_REQUEST_DO_REQUEST FName("SearchingSendFollowRequestDoRequest")

class FMatchmakingAlgorithmStepStateSearchingSendFollowRequestDoRequest : public IMatchmakingAlgorithmStepState
{
public:
    bool bConnected;
    bool bGotResponse;
    TSharedPtr<const FOnlineLobbyId> FollowRequestLobbyId;
    FDelegateHandle OnLobbyUpdateHandle;
    FDelegateHandle OnTimeoutHandle;
    FDelegateHandle OnWaitingForPlayersTimeoutHandle;
    bool bWaitingForUsersToConnect;
    FMatchmakingSuitableMatchResult MatchCandidateForWaiting;

    FMatchmakingAlgorithmStepStateSearchingSendFollowRequestDoRequest()
        : bConnected(false)
        , bGotResponse(false)
        , FollowRequestLobbyId(nullptr)
        , OnLobbyUpdateHandle(FDelegateHandle())
        , OnTimeoutHandle(FDelegateHandle())
        , bWaitingForUsersToConnect(false)
        , MatchCandidateForWaiting(FMatchmakingSuitableMatchResult::Invalid())
    {
    }
};

class FMatchmakingAlgorithmStepSearchingSendFollowRequestDoRequest
    : public IMatchmakingAlgorithmStep<
          FMatchmakingAlgorithmStepStateSearchingSendFollowRequestDoRequest,
          FMatchmakingSuitableMatchResult>,
      public TSharedFromThis<FMatchmakingAlgorithmStepSearchingSendFollowRequestDoRequest>
{
private:
    DECLARE_DELEGATE(FOnContinueFromKick);

    void CleanupAndReturnToSearch(
        const TSharedRef<FMatchmakingEngineTask> &Task,
        const FMatchmakingSuitableMatchResult &MatchCandidate);
    void CleanupAndReturnToSearchAfterDisconnect(
        const FOnlineError &Error,
        const FUniqueNetId &UserId,
        TWeakPtr<FMatchmakingEngineTask> TaskWk,
        FMatchmakingSuitableMatchResult MatchCandidate);
    void CleanupAndReturnToSearchAfterKick(
        TWeakPtr<FMatchmakingEngineTask> TaskWk,
        FMatchmakingSuitableMatchResult MatchCandidate);

    /** When this user has connected to the follow request lobby */
    void OnConnected(
        const FOnlineError &Error,
        const FUniqueNetId &UserId,
        const TSharedPtr<class FOnlineLobby> &Lobby,
        TWeakPtr<FMatchmakingEngineTask> TaskWk,
        FMatchmakingSuitableMatchResult MatchCandidate);
    /** When we have sent out our follow request to the target lobby */
    void OnFollowRequestSent(
        const FOnlineError &Error,
        const FUniqueNetId &UserId,
        TWeakPtr<FMatchmakingEngineTask> TaskWk,
        FMatchmakingSuitableMatchResult MatchCandidate,
        TSharedPtr<const FUniqueNetId> LobbyOwner);
    /** When the target has responded to our follow request */
    void OnLobbyUpdateReceived(
        const FUniqueNetId &UserId,
        const FOnlineLobbyId &LobbyId,
        TWeakPtr<FMatchmakingEngineTask> TaskWk,
        FMatchmakingSuitableMatchResult MatchCandidate);
    /** When we don't get a response from the target within the timeout */
    void OnTimeout(
        TWeakPtr<FMatchmakingEngineTask> TaskWk,
        FMatchmakingSuitableMatchResult MatchCandidate,
        TSharedPtr<const FUniqueNetId> LobbyOwner,
        bool bAllowTimeoutExtension);
    /** When we either have a response or timeout, this handles it before we disconnect from the follow request lobby */
    void OnFollowResponseReceived(
        const TSharedRef<FMatchmakingEngineTask> &Task,
        const FMatchmakingSuitableMatchResult &MatchCandidate,
        bool bAccepted);
    /** When we either have a response or timeout, this handles it after we have disconnected from the follow request
     * lobby */
    void OnFollowResponseReceivedAfterDisconnect(
        const FOnlineError &Error,
        const FUniqueNetId &UserId,
        TWeakPtr<FMatchmakingEngineTask> TaskWk,
        FMatchmakingSuitableMatchResult MatchCandidate,
        bool bAccepted);
    /** When a member has been kicked */
    void OnMemberKickComplete(
        const FOnlineError &Error,
        const FUniqueNetId &UserId,
        TWeakPtr<FMatchmakingEngineTask> TaskWk,
        TSharedRef<const FUniqueNetId> ConnectedUser);
    /** When the follow process is complete, and we need to either search for more players or complete the match */
    void MoveToSearchingOrCompleteMatch(
        const TSharedRef<FMatchmakingEngineTask> &Task,
        const FMatchmakingSuitableMatchResult &MatchCandidate);
    /** When we were waiting for players to join, we ran out of time for them to connect. */
    void OnWaitingForPlayersTimeoutHandle(TWeakPtr<FMatchmakingEngineTask> TaskWk);

    /**
     * Removes the pending batch, kicks the connected users, and then calls the callback if all of the kicking
     * happened successfully.
     */
    void RemovePendingBatchAndKickUsers(
        const TSharedRef<FMatchmakingEngineTask> &Task,
        const FOnContinueFromKick &OnContinue);

public:
    virtual ~FMatchmakingAlgorithmStepSearchingSendFollowRequestDoRequest() override = default;
    UE_NONCOPYABLE(FMatchmakingAlgorithmStepSearchingSendFollowRequestDoRequest);
    FMatchmakingAlgorithmStepSearchingSendFollowRequestDoRequest() = default;

    virtual FName GetName() const override
    {
        return MMSTEP_SEARCHING_SEND_FOLLOW_REQUEST_DO_REQUEST;
    }

    virtual FText GetStatus() const override
    {
        return NSLOCTEXT("Matchmaking", "SearchingSendFollowRequestDoRequest_Status", "Searching for other players");
    }

    virtual FText GetDetail() const override
    {
        return NSLOCTEXT(
            "Matchmaking",
            "SearchingSendFollowRequestDoRequest_Detail",
            "Communicating with other players...");
    }

    virtual TSharedPtr<FMatchmakingAlgorithmStepStateSearchingSendFollowRequestDoRequest> CreateNewState() override;
    virtual void Start(TSharedRef<FMatchmakingEngineTask> Task, FMatchmakingSuitableMatchResult MatchCandidate)
        override;
    virtual bool CanHandleInterrupt(const TSharedRef<FMatchmakingEngineTask> &Task, FName InterruptType) override;
    virtual bool HandleInterrupt(const TSharedRef<FMatchmakingEngineTask> &Task, FName InterruptType) override;
};

#endif

REDPOINT_EOS_CODE_GUARD_END()