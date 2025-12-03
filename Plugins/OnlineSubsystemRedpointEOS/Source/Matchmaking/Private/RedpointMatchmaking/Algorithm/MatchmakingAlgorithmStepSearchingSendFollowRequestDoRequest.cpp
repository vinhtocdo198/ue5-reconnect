// Copyright June Rhodes. All Rights Reserved.

#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepSearchingSendFollowRequestDoRequest.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "../MatchmakingFollowRequest.h"
#include "../MatchmakingFollowResponse.h"
#include "./MatchmakingAlgorithmStepAbandonStart.h"
#include "./MatchmakingAlgorithmStepCompleteMatch.h"
#include "./MatchmakingAlgorithmStepSearching.h"
#include "Containers/Ticker.h"
#include "RedpointMatchmaking/EventHandlers/MatchmakingEventHandlerTeamManagement.h"
#include "RedpointMatchmaking/MatchmakingConstants.h"
#include "RedpointMatchmaking/MatchmakingLog.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

TSharedPtr<FMatchmakingAlgorithmStepStateSearchingSendFollowRequestDoRequest>
FMatchmakingAlgorithmStepSearchingSendFollowRequestDoRequest::CreateNewState()
{
    return MakeShared<FMatchmakingAlgorithmStepStateSearchingSendFollowRequestDoRequest>();
}

void FMatchmakingAlgorithmStepSearchingSendFollowRequestDoRequest::CleanupAndReturnToSearch(
    const TSharedRef<FMatchmakingEngineTask> &Task,
    const FMatchmakingSuitableMatchResult &MatchCandidate)
{
    auto State = this->GetState(*Task);
    if (State->OnTimeoutHandle.IsValid())
    {
        Task->Unschedule.Execute(State->OnTimeoutHandle);
        State->OnTimeoutHandle.Reset();
    }
    if (State->OnLobbyUpdateHandle.IsValid())
    {
        if (auto Pins = Task->PinAndContinue())
        {
            Pins->OnlineLobby->ClearOnLobbyUpdateDelegate_Handle(State->OnLobbyUpdateHandle);
        }
        State->OnLobbyUpdateHandle.Reset();
    }
    State->bWaitingForUsersToConnect = false;
    State->MatchCandidateForWaiting = FMatchmakingSuitableMatchResult::Invalid();
    if (State->OnWaitingForPlayersTimeoutHandle.IsValid())
    {
        Task->Unschedule.Execute(State->OnWaitingForPlayersTimeoutHandle);
        State->OnWaitingForPlayersTimeoutHandle.Reset();
    }
    State->bGotResponse = false;
    if (State->bConnected)
    {
        if (auto Pins = Task->PinAndContinue())
        {
            if (!Pins->OnlineLobby->DisconnectLobby(
                    *Task->Request.UserId,
                    *State->FollowRequestLobbyId,
                    FOnLobbyOperationComplete::CreateSP(
                        this,
                        &FMatchmakingAlgorithmStepSearchingSendFollowRequestDoRequest::
                            CleanupAndReturnToSearchAfterDisconnect,
                        TWeakPtr<FMatchmakingEngineTask>(Task),
                        MatchCandidate)))
            {
                UE_LOG(
                    LogMatchmaking,
                    Error,
                    TEXT("%sCould not disconnect from follow request lobby (CleanupAndReturnToSearch)"),
                    *this->GetLogPrefix(*Task));
            }
            else
            {
                return;
            }
        }
    }

    State->bConnected = false;
    EMIT_TASK_COUNTER_TRACE(Task);
    this->MoveToStep(*Task, MMSTEP_SEARCHING, false);
}

void FMatchmakingAlgorithmStepSearchingSendFollowRequestDoRequest::CleanupAndReturnToSearchAfterDisconnect(
    const FOnlineError &Error,
    const FUniqueNetId &UserId,
    TWeakPtr<FMatchmakingEngineTask> TaskWk,
    FMatchmakingSuitableMatchResult MatchCandidate)
{
    auto Task = TaskWk.Pin();
    if (!Task.IsValid())
    {
        return;
    }

    if (!Task->CurrentStep.IsEqual(MMSTEP_SEARCHING_SEND_FOLLOW_REQUEST_DO_REQUEST))
    {
        return;
    }

    if (!Error.bSucceeded)
    {
        UE_LOG(
            LogMatchmaking,
            Error,
            TEXT("%sCould not disconnect from follow request lobby (CleanupAndReturnToSearchAfterDisconnect): %s"),
            *this->GetLogPrefix(*Task),
            *Error.ToLogString());
    }

    auto State = this->GetState(*Task);
    State->bConnected = false;
    EMIT_TASK_COUNTER_TRACE(Task);
    this->MoveToStep(*Task, MMSTEP_SEARCHING, false);
}

void FMatchmakingAlgorithmStepSearchingSendFollowRequestDoRequest::CleanupAndReturnToSearchAfterKick(
    TWeakPtr<FMatchmakingEngineTask> TaskWk,
    FMatchmakingSuitableMatchResult MatchCandidate)
{
    auto Task = TaskWk.Pin();
    if (!Task.IsValid())
    {
        return;
    }

    if (!Task->CurrentStep.IsEqual(MMSTEP_SEARCHING_SEND_FOLLOW_REQUEST_DO_REQUEST))
    {
        return;
    }

    this->CleanupAndReturnToSearch(Task.ToSharedRef(), MatchCandidate);
}

void FMatchmakingAlgorithmStepSearchingSendFollowRequestDoRequest::OnConnected(
    const FOnlineError &Error,
    const FUniqueNetId &UserId,
    const TSharedPtr<class FOnlineLobby> &Lobby,
    TWeakPtr<FMatchmakingEngineTask> TaskWk,
    FMatchmakingSuitableMatchResult MatchCandidate)
{
    auto Task = TaskWk.Pin();
    if (!Task.IsValid())
    {
        return;
    }

    if (!Task->CurrentStep.IsEqual(MMSTEP_SEARCHING_SEND_FOLLOW_REQUEST_DO_REQUEST))
    {
        return;
    }

    auto State = this->GetState(*Task);

    if (!Error.bSucceeded)
    {
        if (Error.ErrorCode.EndsWith(".not_found"))
        {
            // This can occur with the EOS backend, when we try to connect to a follow request lobby, but it's
            // already gone. Treat it like rejection.
            UE_LOG(
                LogMatchmaking,
                Verbose,
                TEXT("%sFollow request lobby %s disappeared before we could connect to it, treating it as a rejection"),
                *this->GetLogPrefix(*Task),
                *State->FollowRequestLobbyId->ToString());
            Task->ExcludedLobbyIds.Add(*MatchCandidate.CandidateLobbyId);
            this->RemovePendingBatchAndKickUsers(
                Task.ToSharedRef(),
                FOnContinueFromKick::CreateSP(
                    this,
                    &FMatchmakingAlgorithmStepSearchingSendFollowRequestDoRequest::CleanupAndReturnToSearchAfterKick,
                    TWeakPtr<FMatchmakingEngineTask>(Task),
                    MatchCandidate));
            return;
        }

        // We can't connect to this candidate.
        Task->ExcludedLobbyIds.Add(*MatchCandidate.CandidateLobbyId);
        UE_LOG(
            LogMatchmaking,
            Error,
            TEXT("%sCould not connect to follow request lobby %s: %s"),
            *this->GetLogPrefix(*Task),
            *State->FollowRequestLobbyId->ToString(),
            *Error.ToLogString());
        this->RemovePendingBatchAndKickUsers(
            Task.ToSharedRef(),
            FOnContinueFromKick::CreateSP(
                this,
                &FMatchmakingAlgorithmStepSearchingSendFollowRequestDoRequest::CleanupAndReturnToSearchAfterKick,
                TWeakPtr<FMatchmakingEngineTask>(Task),
                MatchCandidate));
        return;
    }

    State->bConnected = true;

    UE_LOG(
        LogMatchmaking,
        Verbose,
        TEXT("%sConnected to follow request lobby %s"),
        *this->GetLogPrefix(*Task),
        *Lobby->Id->ToString());

    if (auto Pins = Task->PinAndContinue())
    {
        State->OnLobbyUpdateHandle =
            Pins->OnlineLobby->AddOnLobbyUpdateDelegate_Handle(FOnLobbyUpdateDelegate::CreateSP(
                this,
                &FMatchmakingAlgorithmStepSearchingSendFollowRequestDoRequest::OnLobbyUpdateReceived,
                TWeakPtr<FMatchmakingEngineTask>(Task),
                MatchCandidate));

        auto MemberTransaction = Pins->OnlineLobby->MakeUpdateLobbyMemberTransaction(
            *Task->Request.UserId,
            *Lobby->Id,
            *Task->Request.UserId);
        MemberTransaction->SetMetadata.Add(
            MatchmakingMemberMetadataFollowRequest,
            FMatchmakingFollowRequest::SerializeFollowRequest(
                *Task->MatchmakingLobbyId,
                ++Task->FollowRequestId,
                Task->Started.ToUnixTimestamp(),
                MatchCandidate.StateHash));

        if (!Pins->OnlineLobby->UpdateMemberSelf(
                *Task->Request.UserId,
                *Lobby->Id,
                *MemberTransaction,
                FOnLobbyOperationComplete::CreateSP(
                    this,
                    &FMatchmakingAlgorithmStepSearchingSendFollowRequestDoRequest::OnFollowRequestSent,
                    TWeakPtr<FMatchmakingEngineTask>(Task),
                    MatchCandidate,
                    Lobby->OwnerId)))
        {
            Task->ExcludedLobbyIds.Add(*MatchCandidate.CandidateLobbyId);
            UE_LOG(
                LogMatchmaking,
                Error,
                TEXT("%sCould not send follow request to lobby %s"),
                *this->GetLogPrefix(*Task),
                *Lobby->Id->ToString());
            this->RemovePendingBatchAndKickUsers(
                Task.ToSharedRef(),
                FOnContinueFromKick::CreateSP(
                    this,
                    &FMatchmakingAlgorithmStepSearchingSendFollowRequestDoRequest::CleanupAndReturnToSearchAfterKick,
                    TWeakPtr<FMatchmakingEngineTask>(Task),
                    MatchCandidate));
            return;
        }
    }
}

void FMatchmakingAlgorithmStepSearchingSendFollowRequestDoRequest::OnFollowRequestSent(
    const FOnlineError &Error,
    const FUniqueNetId &UserId,
    TWeakPtr<FMatchmakingEngineTask> TaskWk,
    FMatchmakingSuitableMatchResult MatchCandidate,
    TSharedPtr<const FUniqueNetId> LobbyOwner)
{
    auto Task = TaskWk.Pin();
    if (!Task.IsValid())
    {
        return;
    }

    if (!Task->CurrentStep.IsEqual(MMSTEP_SEARCHING_SEND_FOLLOW_REQUEST_DO_REQUEST))
    {
        return;
    }

    auto State = this->GetState(*Task);

    if (State->bGotResponse)
    {
        // We already handled the response, we don't need to start the timer.
        UE_LOG(
            LogMatchmaking,
            Verbose,
            TEXT("%sAlready got a response event by the time OnFollowRequestSent was being handled."),
            *this->GetLogPrefix(*Task));
        return;
    }

    if (!Error.bSucceeded)
    {
        if (Error.ErrorCode.EndsWith(".unexpected_error"))
        {
            // This can occur with the EOS backend, when we try to send a follow request to a lobby
            // that no longer exists. We don't want to emit this as an error, since it can happen
            // normally. Instead just treat it like a rejection message.
            UE_LOG(
                LogMatchmaking,
                Verbose,
                TEXT(
                    "%sFollow request lobby %s disappeared before we could send a request, treating it as a rejection"),
                *this->GetLogPrefix(*Task),
                *State->FollowRequestLobbyId->ToString());
            Task->ExcludedLobbyIds.Add(*MatchCandidate.CandidateLobbyId);
            this->RemovePendingBatchAndKickUsers(
                Task.ToSharedRef(),
                FOnContinueFromKick::CreateSP(
                    this,
                    &FMatchmakingAlgorithmStepSearchingSendFollowRequestDoRequest::CleanupAndReturnToSearchAfterKick,
                    TWeakPtr<FMatchmakingEngineTask>(Task),
                    MatchCandidate));
            return;
        }
        else if (Error.ErrorCode.EndsWith(".not_found"))
        {
            // Same as unexpected_error above.
            UE_LOG(
                LogMatchmaking,
                Verbose,
                TEXT(
                    "%sFollow request lobby %s disappeared before we could send a request, treating it as a rejection"),
                *this->GetLogPrefix(*Task),
                *State->FollowRequestLobbyId->ToString());
            Task->ExcludedLobbyIds.Add(*MatchCandidate.CandidateLobbyId);
            this->RemovePendingBatchAndKickUsers(
                Task.ToSharedRef(),
                FOnContinueFromKick::CreateSP(
                    this,
                    &FMatchmakingAlgorithmStepSearchingSendFollowRequestDoRequest::CleanupAndReturnToSearchAfterKick,
                    TWeakPtr<FMatchmakingEngineTask>(Task),
                    MatchCandidate));
            return;
        }
        else if (Error.ErrorCode.EndsWith(".timed_out"))
        {
            // Timed out while trying to update our own metadata for the follow request. Treat it as a rejection.
            UE_LOG(
                LogMatchmaking,
                Verbose,
                TEXT("%sTimed out while trying to send follow request to lobby %s, treating it as a rejection"),
                *this->GetLogPrefix(*Task),
                *State->FollowRequestLobbyId->ToString());
            Task->ExcludedLobbyIds.Add(*MatchCandidate.CandidateLobbyId);
            this->RemovePendingBatchAndKickUsers(
                Task.ToSharedRef(),
                FOnContinueFromKick::CreateSP(
                    this,
                    &FMatchmakingAlgorithmStepSearchingSendFollowRequestDoRequest::CleanupAndReturnToSearchAfterKick,
                    TWeakPtr<FMatchmakingEngineTask>(Task),
                    MatchCandidate));
            return;
        }

        // We can't update our attribute data.
        Task->ExcludedLobbyIds.Add(*MatchCandidate.CandidateLobbyId);
        UE_LOG(
            LogMatchmaking,
            Error,
            TEXT("%sCould not send follow request to lobby %s: %s"),
            *this->GetLogPrefix(*Task),
            *State->FollowRequestLobbyId->ToString(),
            *Error.ToLogString());
        this->RemovePendingBatchAndKickUsers(
            Task.ToSharedRef(),
            FOnContinueFromKick::CreateSP(
                this,
                &FMatchmakingAlgorithmStepSearchingSendFollowRequestDoRequest::CleanupAndReturnToSearchAfterKick,
                TWeakPtr<FMatchmakingEngineTask>(Task),
                MatchCandidate));
        return;
    }

    UE_LOG(
        LogMatchmaking,
        Verbose,
        TEXT("%sSuccessfully sent follow request to follow request lobby."),
        *this->GetLogPrefix(*Task));

    State->OnTimeoutHandle = Task->Schedule.Execute(
        MatchmakingTimingFollowRequestTimeout,
        FMatchmakingEngineTaskScheduleCallback::CreateSP(
            this,
            &FMatchmakingAlgorithmStepSearchingSendFollowRequestDoRequest::OnTimeout,
            TWeakPtr<FMatchmakingEngineTask>(Task),
            MatchCandidate,
            MoveTemp(LobbyOwner),
            true));
}

void FMatchmakingAlgorithmStepSearchingSendFollowRequestDoRequest::OnLobbyUpdateReceived(
    const FUniqueNetId &UserId,
    const FOnlineLobbyId &LobbyId,
    TWeakPtr<FMatchmakingEngineTask> TaskWk,
    FMatchmakingSuitableMatchResult MatchCandidate)
{
    auto Task = TaskWk.Pin();
    if (!Task.IsValid())
    {
        return;
    }

    if (!Task->CurrentStep.IsEqual(MMSTEP_SEARCHING_SEND_FOLLOW_REQUEST_DO_REQUEST))
    {
        return;
    }

    auto State = this->GetState(*Task);

    // We only care about lobby updates for our follow request lobby.
    if (LobbyId != *State->FollowRequestLobbyId)
    {
        return;
    }

    // Try to receive the response for our particular user - the follow request lobby
    // might be responding to multiple requests, so it's important to make sure ours
    // has been responded to be before we actually mark ourselves as having received
    // a response.
    if (auto Pins = Task->PinAndContinue())
    {
        FVariantData Response;
        if (!Pins->OnlineLobby->GetLobbyMetadataValue(
                *Task->Request.UserId,
                *State->FollowRequestLobbyId,
                FString::Printf(
                    TEXT("%s.%s"),
                    MatchmakingMemberMetadataFollowResponse,
                    *Task->Request.UserId->ToString()),
                Response) ||
            Response.GetType() != EOnlineKeyValuePairDataType::String)
        {
            // We don't have a response yet.
            return;
        }

        int32 ReceivedRequestId;
        bool ReceivedAccept;
        if (!FMatchmakingFollowResponse::TryDeserializeFollowResponse(
                Pins->OnlineLobby,
                Response,
                ReceivedRequestId,
                ReceivedAccept))
        {
            // Not valid format.
            return;
        }

        if (ReceivedRequestId != Task->FollowRequestId)
        {
            // Not a response for our particular request (might be a response to an older request from us).
            UE_LOG(
                LogMatchmaking,
                Verbose,
                TEXT("%sIgnoring response to follow request ID %d as we're expecting a response to follow request ID "
                     "%d"),
                *this->GetLogPrefix(*Task),
                ReceivedRequestId,
                Task->FollowRequestId);
            return;
        }

        State->bGotResponse = true;
        if (State->OnTimeoutHandle.IsValid())
        {
            Task->Unschedule.Execute(State->OnTimeoutHandle);
            State->OnTimeoutHandle.Reset();
        }
        Pins->OnlineLobby->ClearOnLobbyUpdateDelegate_Handle(State->OnLobbyUpdateHandle);
        State->OnLobbyUpdateHandle.Reset();

        if (ReceivedAccept)
        {
            UE_LOG(
                LogMatchmaking,
                Verbose,
                TEXT("%sGot follow response from lobby %s: Accepted"),
                *this->GetLogPrefix(*Task),
                *State->FollowRequestLobbyId->ToString());
        }
        else
        {
            UE_LOG(
                LogMatchmaking,
                Verbose,
                TEXT("%sGot follow response from lobby %s: Rejected"),
                *this->GetLogPrefix(*Task),
                *State->FollowRequestLobbyId->ToString());
        }

        this->OnFollowResponseReceived(Task.ToSharedRef(), MatchCandidate, ReceivedAccept);
    }
}

void FMatchmakingAlgorithmStepSearchingSendFollowRequestDoRequest::OnTimeout(
    TWeakPtr<FMatchmakingEngineTask> TaskWk,
    FMatchmakingSuitableMatchResult MatchCandidate,
    TSharedPtr<const FUniqueNetId> LobbyOwner,
    bool bAllowTimeoutExtension)
{
    auto Task = TaskWk.Pin();
    if (!Task.IsValid())
    {
        return;
    }

    if (!Task->CurrentStep.IsEqual(MMSTEP_SEARCHING_SEND_FOLLOW_REQUEST_DO_REQUEST))
    {
        return;
    }

    if (auto Pins = Task->PinAndContinue())
    {
        auto State = this->GetState(*Task);
        if (State->bGotResponse)
        {
            // This has already been handled.
            return;
        }

        if (!State->bGotResponse && Task->TeamManager->IsConnected(*LobbyOwner) && bAllowTimeoutExtension)
        {
            // Handle a rare race condition where the target is about to accept (they have
            // successfully connected), but their acceptance message in the lobby metadata hasn't propagated
            // to us yet.
            //
            // We reschedule the timeout for just a little bit longer to allow that notification to come
            // in for us, but only once (in case the target is stalled while connected to us).
            State->OnTimeoutHandle = Task->Schedule.Execute(
                MatchmakingTimingFollowRequestTimeout,
                FMatchmakingEngineTaskScheduleCallback::CreateSP(
                    this,
                    &FMatchmakingAlgorithmStepSearchingSendFollowRequestDoRequest::OnTimeout,
                    TWeakPtr<FMatchmakingEngineTask>(Task),
                    MatchCandidate,
                    LobbyOwner,
                    false));
            return;
        }

        Task->ExcludedLobbyIds.Add(*MatchCandidate.CandidateLobbyId);

        UE_LOG(
            LogMatchmaking,
            Verbose,
            TEXT("%sTimed out while waiting for follow response from lobby %s"),
            *this->GetLogPrefix(*Task),
            *State->FollowRequestLobbyId->ToString());

        if (State->OnTimeoutHandle.IsValid())
        {
            Task->Unschedule.Execute(State->OnTimeoutHandle);
            State->OnTimeoutHandle.Reset();
        }
        Pins->OnlineLobby->ClearOnLobbyUpdateDelegate_Handle(State->OnLobbyUpdateHandle);
        State->OnLobbyUpdateHandle.Reset();

        this->RemovePendingBatchAndKickUsers(
            Task.ToSharedRef(),
            FOnContinueFromKick::CreateLambda(
                [WeakThis = TWeakPtr<FMatchmakingAlgorithmStepSearchingSendFollowRequestDoRequest>(this->AsShared()),
                 WeakTask = TWeakPtr<FMatchmakingEngineTask>(Task),
                 MatchCandidate]() {
                    if (auto This = WeakThis.Pin())
                    {
                        if (auto TaskPinned = WeakTask.Pin())
                        {
                            This->OnFollowResponseReceived(TaskPinned.ToSharedRef(), MatchCandidate, false);
                        }
                    }
                }));
    }

    return;
}

void FMatchmakingAlgorithmStepSearchingSendFollowRequestDoRequest::OnFollowResponseReceived(
    const TSharedRef<FMatchmakingEngineTask> &Task,
    const FMatchmakingSuitableMatchResult &MatchCandidate,
    bool bAccepted)
{
    if (auto Pins = Task->PinAndContinue())
    {
        auto State = this->GetState(*Task);

        // Mark as false so that if DisconnectLobby calls it's callback inline, when
        // OnFollowResponseReceivedAfterDisconnect runs CleanupAndReturnToSearch it doesn't try to disconnect a second
        // time.
        State->bConnected = false;

        if (!Pins->OnlineLobby->DisconnectLobby(
                *Task->Request.UserId,
                *State->FollowRequestLobbyId,
                FOnLobbyOperationComplete::CreateSP(
                    this,
                    &FMatchmakingAlgorithmStepSearchingSendFollowRequestDoRequest::
                        OnFollowResponseReceivedAfterDisconnect,
                    TWeakPtr<FMatchmakingEngineTask>(Task),
                    MatchCandidate,
                    bAccepted)))
        {
            // NOTE: We are effectively dropping the response in bAccepted here and ignoring it. Is that the right thing
            // to do?

            // We didn't disconnect properly.
            State->bConnected = true;

            Task->ExcludedLobbyIds.Add(*MatchCandidate.CandidateLobbyId);

            UE_LOG(
                LogMatchmaking,
                Verbose,
                TEXT("%sCould not disconnect from follow request lobby (OnFollowResponseReceived)"),
                *this->GetLogPrefix(*Task));

            this->RemovePendingBatchAndKickUsers(
                Task,
                FOnContinueFromKick::CreateSP(
                    this,
                    &FMatchmakingAlgorithmStepSearchingSendFollowRequestDoRequest::CleanupAndReturnToSearchAfterKick,
                    TWeakPtr<FMatchmakingEngineTask>(Task),
                    MatchCandidate));

            return;
        }
    }
}

void FMatchmakingAlgorithmStepSearchingSendFollowRequestDoRequest::OnFollowResponseReceivedAfterDisconnect(
    const FOnlineError &Error,
    const FUniqueNetId &UserId,
    TWeakPtr<FMatchmakingEngineTask> TaskWk,
    FMatchmakingSuitableMatchResult MatchCandidate,
    bool bAccepted)
{
    auto Task = TaskWk.Pin();
    if (!Task.IsValid())
    {
        return;
    }

    if (!Task->CurrentStep.IsEqual(MMSTEP_SEARCHING_SEND_FOLLOW_REQUEST_DO_REQUEST))
    {
        return;
    }

    auto State = this->GetState(*Task);

    if (!Error.bSucceeded)
    {
        if (Error.ErrorCode.EndsWith(".timed_out") || Error.ErrorCode.EndsWith(".already_pending"))
        {
            if (auto Pins = Task->PinAndContinue())
            {
                // Retry disconnecting from the lobby.
                if (Pins->OnlineLobby->DisconnectLobby(
                        *Task->Request.UserId,
                        *State->FollowRequestLobbyId,
                        FOnLobbyOperationComplete::CreateSP(
                            this,
                            &FMatchmakingAlgorithmStepSearchingSendFollowRequestDoRequest::
                                OnFollowResponseReceivedAfterDisconnect,
                            TWeakPtr<FMatchmakingEngineTask>(Task),
                            MatchCandidate,
                            bAccepted)))
                {
                    // The operation has been retried.
                    return;
                }
                else
                {
                    // Otherwise, let matchmaking continue since we probably did disconnect anyway.
                }
            }
        }
        else
        {
            // NOTE: We are effectively dropping the response in bAccepted here and ignoring it. Is that the right thing
            // to do?

            // We didn't disconnect properly.
            State->bConnected = true;

            Task->ExcludedLobbyIds.Add(*MatchCandidate.CandidateLobbyId);

            UE_LOG(
                LogMatchmaking,
                Error,
                TEXT("%sCould not disconnect from follow request lobby: %s (OnFollowResponseReceivedAfterDisconnect)"),
                *this->GetLogPrefix(*Task),
                *Error.ToLogString());

            this->RemovePendingBatchAndKickUsers(
                Task.ToSharedRef(),
                FOnContinueFromKick::CreateSP(
                    this,
                    &FMatchmakingAlgorithmStepSearchingSendFollowRequestDoRequest::CleanupAndReturnToSearchAfterKick,
                    TWeakPtr<FMatchmakingEngineTask>(Task),
                    MatchCandidate));
            return;
        }
    }

    if (!bAccepted)
    {
        // NOTE: We do not exclude the lobby here, because the candidate lobby might just have been busy when we
        // made the request and it might become available later.

        UE_LOG(
            LogMatchmaking,
            Verbose,
            TEXT("%sFollow request was refused or timed out, going back to doing more searching"),
            *this->GetLogPrefix(*Task));
        Task->RejectedRequestsPenaltyCount++;
        this->RemovePendingBatchAndKickUsers(
            Task.ToSharedRef(),
            FOnContinueFromKick::CreateSP(
                this,
                &FMatchmakingAlgorithmStepSearchingSendFollowRequestDoRequest::CleanupAndReturnToSearchAfterKick,
                TWeakPtr<FMatchmakingEngineTask>(Task),
                MatchCandidate));
        return;
    }

    // Our follow request was accepted, which means that match lobby is merging
    // into ours.
    if (!Task->TeamManager->HasPendingBatch() || Task->TeamManager->IsPendingBatchReady())
    {
        // All users already joined - this is the case if the lobby that was
        // merged into ours only contained a single player (the host of that
        // lobby, who would have already joined before accepting in order to
        // test the ID/secret).
        UE_LOG(
            LogMatchmaking,
            Verbose,
            TEXT("%sFollow request was accepted, all pending players already "
                 "joined"),
            *this->GetLogPrefix(*Task));
        if (Task->TeamManager->HasPendingBatch())
        {
            Task->TeamManager->FinalizePendingBatch();
        }
        this->MoveToSearchingOrCompleteMatch(Task.ToSharedRef(), MatchCandidate);
        return;
    }

    UE_LOG(
        LogMatchmaking,
        Verbose,
        TEXT("%sFollow request was accepted, now waiting for players to "
             "join:"),
        *this->GetLogPrefix(*Task));
    check(!Task->TeamManager->IsPendingBatchReady());
    for (const auto &PendingUserId : Task->TeamManager->GetPendingNotConnectedUsers())
    {
        UE_LOG(LogMatchmaking, Verbose, TEXT("%s - %s"), *this->GetLogPrefix(*Task), *PendingUserId->ToString());
    }

    State->bWaitingForUsersToConnect = true;
    State->MatchCandidateForWaiting = MatchCandidate;

    State->OnWaitingForPlayersTimeoutHandle = Task->Schedule.Execute(
        MatchmakingTimingWaitForPlayersTimeout,
        FMatchmakingEngineTaskScheduleCallback::CreateSP(
            this,
            &FMatchmakingAlgorithmStepSearchingSendFollowRequestDoRequest::OnWaitingForPlayersTimeoutHandle,
            TWeakPtr<FMatchmakingEngineTask>(Task)));
}

void FMatchmakingAlgorithmStepSearchingSendFollowRequestDoRequest::OnWaitingForPlayersTimeoutHandle(
    TWeakPtr<FMatchmakingEngineTask> TaskWk)
{
    // We ran out of time waiting for players. Restart matchmaking.
    if (auto Task = TaskWk.Pin())
    {
        if (Task.IsValid() && Task->CurrentStep.IsEqual(MMSTEP_SEARCHING_SEND_FOLLOW_REQUEST_DO_REQUEST))
        {
            this->MoveToStep(*Task, MMSTEP_ABANDON_START, EMatchmakingAbandonmentType::Restart);
        }
    }
}

void FMatchmakingAlgorithmStepSearchingSendFollowRequestDoRequest::OnMemberKickComplete(
    const FOnlineError &Error,
    const FUniqueNetId &UserId,
    TWeakPtr<FMatchmakingEngineTask> TaskWk,
    TSharedRef<const FUniqueNetId> ConnectedUser)
{
    auto Task = TaskWk.Pin();
    if (!Task.IsValid() || !Task->CurrentStep.IsEqual(MMSTEP_SEARCHING_SEND_FOLLOW_REQUEST_DO_REQUEST))
    {
        return;
    }

    if (!Error.bSucceeded)
    {
        if (Error.ErrorCode.EndsWith(".timed_out") || Error.ErrorCode.EndsWith(".already_pending"))
        {
            if (auto Pins = Task->PinAndContinue())
            {
                if (!Pins->OnlineLobby->KickMember(
                        *Task->Request.UserId,
                        *Task->MatchmakingLobbyId,
                        *ConnectedUser,
                        FOnLobbyOperationComplete::CreateSP(
                            this,
                            &FMatchmakingAlgorithmStepSearchingSendFollowRequestDoRequest::OnMemberKickComplete,
                            TWeakPtr<FMatchmakingEngineTask>(Task),
                            ConnectedUser)))
                {
                    this->RestartableError(
                        *Task,
                        TEXT("Can't kick connected pending user from lobby during retry for timed_out."));
                }
                return;
            }
        }

        UE_LOG(LogMatchmaking, Error, TEXT("Failed to kick member from lobby: %s"), *Error.ToLogString())
    }
}

void FMatchmakingAlgorithmStepSearchingSendFollowRequestDoRequest::MoveToSearchingOrCompleteMatch(
    const TSharedRef<FMatchmakingEngineTask> &Task,
    const FMatchmakingSuitableMatchResult &MatchCandidate)
{
    if (Task->TeamManager->IsMatchComplete())
    {
        // Match is full, complete it.
        UE_LOG(LogMatchmaking, Verbose, TEXT("%sMatch is now complete"), *this->GetLogPrefix(*Task));
        auto State = this->GetState(*Task);
        State->bConnected = false;
        State->bGotResponse = false;
        EMIT_TASK_COUNTER_TRACE(Task);
        this->MoveToStep(*Task, MMSTEP_COMPLETE_MATCH);
    }
    else
    {
        // Need to search for more players.
        UE_LOG(LogMatchmaking, Verbose, TEXT("%sReturning to search for more players"), *this->GetLogPrefix(*Task));
        this->CleanupAndReturnToSearch(Task, MatchCandidate);
    }
}

void FMatchmakingAlgorithmStepSearchingSendFollowRequestDoRequest::RemovePendingBatchAndKickUsers(
    const TSharedRef<FMatchmakingEngineTask> &Task,
    const FOnContinueFromKick &OnContinue)
{
    if (!Task->TeamManager->HasPendingBatch())
    {
        OnContinue.ExecuteIfBound();
        return;
    }

    TArray<TSharedRef<const FUniqueNetId>> ConnectedUsersToKick;
    Task->TeamManager->RemovePendingBatch(ConnectedUsersToKick);

    if (ConnectedUsersToKick.Num() == 0)
    {
        OnContinue.ExecuteIfBound();
        return;
    }

    if (auto Pins = Task->PinAndContinue())
    {
        for (const auto &ConnectedUser : ConnectedUsersToKick)
        {
            UE_LOG(
                LogMatchmaking,
                Verbose,
                TEXT("%sKicking pending connected member because the pending batch was removed: %s"),
                *this->GetLogPrefix(*Task),
                *ConnectedUser->ToString());

            if (!Pins->OnlineLobby->KickMember(
                    *Task->Request.UserId,
                    *Task->MatchmakingLobbyId,
                    *ConnectedUser,
                    FOnLobbyOperationComplete::CreateSP(
                        this,
                        &FMatchmakingAlgorithmStepSearchingSendFollowRequestDoRequest::OnMemberKickComplete,
                        TWeakPtr<FMatchmakingEngineTask>(Task),
                        ConnectedUser)))
            {
                this->RestartableError(
                    *Task,
                    TEXT("Can't kick connected pending user from lobby in response to RemovePendingBatch."));
            }
        }

        // TODO: Pretty sure we should wait until all the required members are kicked before proceeding.
    }

    OnContinue.ExecuteIfBound();
}

void FMatchmakingAlgorithmStepSearchingSendFollowRequestDoRequest::Start(
    TSharedRef<FMatchmakingEngineTask> Task,
    FMatchmakingSuitableMatchResult MatchCandidate)
{
    auto State = this->GetState(*Task);
    State->bConnected = false;
    State->bGotResponse = false;
    State->bWaitingForUsersToConnect = false;
    State->OnLobbyUpdateHandle.Reset();
    State->OnTimeoutHandle.Reset();
    State->OnWaitingForPlayersTimeoutHandle.Reset();
    State->MatchCandidateForWaiting = FMatchmakingSuitableMatchResult::Invalid();

    // Connect to the remote lobby.
    if (auto Pins = Task->PinAndContinue())
    {
        // Get the "follow request" lobby ID from the candidate lobby.
        FVariantData FollowRequestLobbyId;
        if (!Pins->OnlineLobby->GetLobbyMetadataValue(
                *Task->Request.UserId,
                *MatchCandidate.CandidateLobbyId,
                MatchmakingMetadataFollowRequestLobbyId,
                FollowRequestLobbyId) ||
            FollowRequestLobbyId.GetType() != EOnlineKeyValuePairDataType::String)
        {
            // We can't get the follow request ID from this candidate.
            Task->ExcludedLobbyIds.Add(*MatchCandidate.CandidateLobbyId);
            UE_LOG(
                LogMatchmaking,
                Error,
                TEXT("%sCould not retrieve follow request lobby ID for candidate lobby %s"),
                *this->GetLogPrefix(*Task),
                *Task->MatchmakingLobbyId->ToString());
            this->RemovePendingBatchAndKickUsers(
                Task,
                FOnContinueFromKick::CreateSP(
                    this,
                    &FMatchmakingAlgorithmStepSearchingSendFollowRequestDoRequest::CleanupAndReturnToSearchAfterKick,
                    TWeakPtr<FMatchmakingEngineTask>(Task),
                    MatchCandidate));
            return;
        }

        FString FollowRequestLobbyIdStr;
        FollowRequestLobbyId.GetValue(FollowRequestLobbyIdStr);
        TSharedPtr<const FOnlineLobbyId> FollowRequestLobbyIdPtr =
            Pins->OnlineLobby->ParseSerializedLobbyId(FollowRequestLobbyIdStr);
        if (!FollowRequestLobbyIdPtr.IsValid())
        {
            Task->ExcludedLobbyIds.Add(*MatchCandidate.CandidateLobbyId);
            UE_LOG(
                LogMatchmaking,
                Error,
                TEXT("%sCould not parse follow request lobby ID for candidate lobby %s"),
                *this->GetLogPrefix(*Task),
                *Task->MatchmakingLobbyId->ToString());
            this->RemovePendingBatchAndKickUsers(
                Task,
                FOnContinueFromKick::CreateSP(
                    this,
                    &FMatchmakingAlgorithmStepSearchingSendFollowRequestDoRequest::CleanupAndReturnToSearchAfterKick,
                    TWeakPtr<FMatchmakingEngineTask>(Task),
                    MatchCandidate));
            return;
        }

        UE_LOG(
            LogMatchmaking,
            Verbose,
            TEXT("%sConnecting to target lobby %s"),
            *this->GetLogPrefix(*Task),
            *FollowRequestLobbyIdPtr->ToString());
        State->FollowRequestLobbyId = FollowRequestLobbyIdPtr;

        if (!Pins->OnlineLobby->ConnectLobby(
                *Task->Request.UserId,
                *FollowRequestLobbyIdPtr,
                FOnLobbyCreateOrConnectComplete::CreateSP(
                    this,
                    &FMatchmakingAlgorithmStepSearchingSendFollowRequestDoRequest::OnConnected,
                    TWeakPtr<FMatchmakingEngineTask>(Task),
                    MatchCandidate)))
        {
            // We can't connect to this candidate.
            Task->ExcludedLobbyIds.Add(*MatchCandidate.CandidateLobbyId);
            UE_LOG(
                LogMatchmaking,
                Error,
                TEXT("%sCould not connect to follow request lobby %s"),
                *this->GetLogPrefix(*Task),
                *FollowRequestLobbyIdPtr->ToString());
            this->RemovePendingBatchAndKickUsers(
                Task,
                FOnContinueFromKick::CreateSP(
                    this,
                    &FMatchmakingAlgorithmStepSearchingSendFollowRequestDoRequest::CleanupAndReturnToSearchAfterKick,
                    TWeakPtr<FMatchmakingEngineTask>(Task),
                    MatchCandidate));
            return;
        }
    }
}

bool FMatchmakingAlgorithmStepSearchingSendFollowRequestDoRequest::CanHandleInterrupt(
    const TSharedRef<FMatchmakingEngineTask> &Task,
    FName InterruptType)
{
    return InterruptType.IsEqual(MMINTERRUPT_ALL_PLAYERS_HAVE_CONNECTED);
}

bool FMatchmakingAlgorithmStepSearchingSendFollowRequestDoRequest::HandleInterrupt(
    const TSharedRef<FMatchmakingEngineTask> &Task,
    FName InterruptType)
{
    if (InterruptType.IsEqual(MMINTERRUPT_ALL_PLAYERS_HAVE_CONNECTED))
    {
        auto State = this->GetState(*Task);
        if (State->bWaitingForUsersToConnect)
        {
            auto MatchCandidate = State->MatchCandidateForWaiting;
            State->bWaitingForUsersToConnect = false;
            State->MatchCandidateForWaiting = FMatchmakingSuitableMatchResult::Invalid();
            if (State->OnWaitingForPlayersTimeoutHandle.IsValid())
            {
                Task->Unschedule.Execute(State->OnWaitingForPlayersTimeoutHandle);
                State->OnWaitingForPlayersTimeoutHandle.Reset();
            }

            UE_LOG(
                LogMatchmaking,
                Verbose,
                TEXT("%sAll pending users finished connecting from follow request"),
                *this->GetLogPrefix(*Task));
            this->MoveToSearchingOrCompleteMatch(Task, MatchCandidate);
        }
        return true;
    }

    return false;
}

#endif

REDPOINT_EOS_CODE_GUARD_END()