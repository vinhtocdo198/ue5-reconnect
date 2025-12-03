// Copyright June Rhodes. All Rights Reserved.

#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepIncomingFollowRequestAccept.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "../MatchmakingFollowRequest.h"
#include "../MatchmakingFollowResponse.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepSearching.h"
#include "RedpointMatchmaking/MatchmakingConstants.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

TSharedPtr<FMatchmakingAlgorithmStepStateIncomingFollowRequestAccept>
FMatchmakingAlgorithmStepIncomingFollowRequestAccept::CreateNewState()
{
    return MakeShared<FMatchmakingAlgorithmStepStateIncomingFollowRequestAccept>();
}

void FMatchmakingAlgorithmStepIncomingFollowRequestAccept::OnDeletedFollowRequestLobby(
    const FOnlineError &Error,
    const FUniqueNetId &UserId,
    TWeakPtr<FMatchmakingEngineTask> TaskWk,
    TSharedPtr<const FOnlineLobbyId> TargetLobbyId,
    TSharedRef<const FUniqueNetId> TargetMemberId)
{
    auto Task = TaskWk.Pin();
    if (!Task.IsValid())
    {
        return;
    }

    if (!Task->CurrentStep.IsEqual(MMSTEP_INCOMING_FOLLOW_REQUEST_ACCEPT))
    {
        return;
    }

    EMIT_TASK_COUNTER_TRACE(Task);

    if (!Error.bSucceeded)
    {
        if (Error.ErrorCode.EndsWith(".timed_out") || Error.ErrorCode.EndsWith(".already_pending"))
        {
            if (auto Pins = Task->PinAndContinue())
            {
                if (!Pins->OnlineLobby->DeleteLobby(
                        UserId,
                        *Task->MatchmakingFollowRequestLobbyId,
                        FOnLobbyOperationComplete::CreateSP(
                            this,
                            &FMatchmakingAlgorithmStepIncomingFollowRequestAccept::OnDeletedFollowRequestLobby,
                            TWeakPtr<FMatchmakingEngineTask>(Task),
                            MoveTemp(TargetLobbyId),
                            MoveTemp(TargetMemberId))))
                {
                    UE_LOG(
                        LogMatchmaking,
                        Warning,
                        TEXT("%sUnable to delete matchmaking follow request lobby after we started following another "
                             "host."),
                        *this->GetLogPrefix(*Task));
                }
                return;
            }
        }

        UE_LOG(
            LogMatchmaking,
            Warning,
            TEXT("%sUnable to delete matchmaking follow request lobby after we started following another host."),
            *this->GetLogPrefix(*Task));
        EMIT_TASK_COUNTER_TRACE(Task);
    }
    else
    {
        // The follow request lobby no longer exists.
        Task->MatchmakingFollowRequestLobbyId.Reset();
        EMIT_TASK_COUNTER_TRACE(Task);
    }

    EMIT_TASK_COUNTER_TRACE(Task);
    UE_LOG(
        LogMatchmaking,
        Verbose,
        TEXT("%sMove to follow other host complete; now acting as client."),
        *this->GetLogPrefix(*Task));
    EMIT_TASK_COUNTER_TRACE(Task);

    // Update the task state so that the ClientFollowing event handler can work.
    Task->MatchmakingLobbyId = MoveTemp(TargetLobbyId);

    EMIT_TASK_COUNTER_TRACE(Task);

    // Remove the TeamManager from the task, as we do not have a TeamManager on clients.
    Task->TeamManager.Reset();

    EMIT_TASK_COUNTER_TRACE(Task);

    this->MoveToStep(*Task, MMSTEP_FOLLOW_HOST);
}

void FMatchmakingAlgorithmStepIncomingFollowRequestAccept::OnDisconnectedFromMatchmakingLobby(
    const FOnlineError &Error,
    const FUniqueNetId &UserId,
    TWeakPtr<FMatchmakingEngineTask> TaskWk,
    TSharedPtr<const FOnlineLobbyId> TargetLobbyId,
    TSharedRef<const FUniqueNetId> TargetMemberId)
{
    auto Task = TaskWk.Pin();
    if (!Task.IsValid())
    {
        return;
    }

    if (!Task->CurrentStep.IsEqual(MMSTEP_INCOMING_FOLLOW_REQUEST_ACCEPT))
    {
        return;
    }

    EMIT_TASK_COUNTER_TRACE(Task);

    if (!Error.bSucceeded)
    {
        if (Error.ErrorCode.EndsWith(".timed_out") || Error.ErrorCode.EndsWith(".already_pending"))
        {
            // Retry on timeout.
            if (auto Pins = Task->PinAndContinue())
            {
                if (!Pins->OnlineLobby->DisconnectLobby(
                        UserId,
                        *Task->MatchmakingLobbyId,
                        FOnLobbyOperationComplete::CreateSP(
                            this,
                            &FMatchmakingAlgorithmStepIncomingFollowRequestAccept::OnDisconnectedFromMatchmakingLobby,
                            TWeakPtr<FMatchmakingEngineTask>(Task),
                            MoveTemp(TargetLobbyId),
                            MoveTemp(TargetMemberId))))
                {
                    UE_LOG(
                        LogMatchmaking,
                        Warning,
                        TEXT("%sUnable to disconnect from matchmaking lobby after we started following another host on "
                             "retry for timed_out."),
                        *this->GetLogPrefix(*Task));
                    EMIT_TASK_COUNTER_TRACE(Task);
                }
                return;
            }
        }

        UE_LOG(
            LogMatchmaking,
            Warning,
            TEXT("%sUnable to disconnect from matchmaking lobby after we started following another host."),
            *this->GetLogPrefix(*Task));
        EMIT_TASK_COUNTER_TRACE(Task);
    }
    else
    {
        // We are no longer associated with the original lobby.
        Task->MatchmakingLobbyId.Reset();
        EMIT_TASK_COUNTER_TRACE(Task);
    }

    if (auto Pins = Task->PinAndContinue())
    {
        EMIT_TASK_COUNTER_TRACE(Task);
        if (!Pins->OnlineLobby->DeleteLobby(
                UserId,
                *Task->MatchmakingFollowRequestLobbyId,
                FOnLobbyOperationComplete::CreateSP(
                    this,
                    &FMatchmakingAlgorithmStepIncomingFollowRequestAccept::OnDeletedFollowRequestLobby,
                    TWeakPtr<FMatchmakingEngineTask>(Task),
                    MoveTemp(TargetLobbyId),
                    MoveTemp(TargetMemberId))))
        {
            EMIT_TASK_COUNTER_TRACE(Task);
            UE_LOG(
                LogMatchmaking,
                Warning,
                TEXT("%sUnable to delete matchmaking follow request lobby after we started following another host."),
                *this->GetLogPrefix(*Task));
        }
        EMIT_TASK_COUNTER_TRACE(Task);
    }
}

void FMatchmakingAlgorithmStepIncomingFollowRequestAccept::OnMatchmakingLobbyUpdated(
    const FOnlineError &Error,
    const FUniqueNetId &UserId,
    TWeakPtr<FMatchmakingEngineTask> TaskWk,
    TSharedPtr<const FOnlineLobbyId> TargetLobbyId,
    TSharedRef<const FUniqueNetId> TargetMemberId)
{
    auto Task = TaskWk.Pin();
    if (!Task.IsValid())
    {
        return;
    }

    if (!Task->CurrentStep.IsEqual(MMSTEP_INCOMING_FOLLOW_REQUEST_ACCEPT))
    {
        return;
    }

    EMIT_TASK_COUNTER_TRACE(Task);

    if (!Error.bSucceeded)
    {
        if (Error.ErrorCode.EndsWith(".timed_out") || Error.ErrorCode.EndsWith(".already_pending"))
        {
            // Retry on timeout.
            if (auto Pins = Task->PinAndContinue())
            {
                auto Txn =
                    Pins->OnlineLobby->MakeUpdateLobbyTransaction(*Task->Request.UserId, *Task->MatchmakingLobbyId);
                Txn->Locked = true;
                Txn->SetMetadata.Add(MatchmakingMetadataLobbyStatus, MatchmakingStatusFollowing);
                Txn->SetMetadata.Add(MatchmakingMetadataLobbyHeartbeatTimestamp, FDateTime::UtcNow().ToUnixTimestamp());
                Txn->SetMetadata.Add(MatchmakingMetadataFollowTargetLobbyId, TargetLobbyId->ToString());

                if (!Pins->OnlineLobby->UpdateLobby(
                        *Task->Request.UserId,
                        *Task->MatchmakingLobbyId,
                        *Txn,
                        FOnLobbyOperationComplete::CreateSP(
                            this,
                            &FMatchmakingAlgorithmStepIncomingFollowRequestAccept::OnMatchmakingLobbyUpdated,
                            TWeakPtr<FMatchmakingEngineTask>(Task),
                            TargetLobbyId,
                            MoveTemp(TargetMemberId))))
                {
                    UE_LOG(
                        LogMatchmaking,
                        Error,
                        TEXT("%sUnable to notify other clients they should join the target lobby on retry timed_out, "
                             "but we have already accepted the follow request! Expect things to break."),
                        *this->GetLogPrefix(*Task));
                }
            }
            return;
        }

        UE_LOG(
            LogMatchmaking,
            Error,
            TEXT("%sUnable to notify other clients they should join the target lobby, but we have already accepted "
                 "the follow request! Expect things to break."),
            *this->GetLogPrefix(*Task));
        EMIT_TASK_COUNTER_TRACE(Task);
    }

    EMIT_TASK_COUNTER_TRACE(Task);
    if (auto Pins = Task->PinAndContinue())
    {
        EMIT_TASK_COUNTER_TRACE(Task);
        if (!Pins->OnlineLobby->DisconnectLobby(
                UserId,
                *Task->MatchmakingLobbyId,
                FOnLobbyOperationComplete::CreateSP(
                    this,
                    &FMatchmakingAlgorithmStepIncomingFollowRequestAccept::OnDisconnectedFromMatchmakingLobby,
                    TWeakPtr<FMatchmakingEngineTask>(Task),
                    MoveTemp(TargetLobbyId),
                    MoveTemp(TargetMemberId))))
        {
            UE_LOG(
                LogMatchmaking,
                Warning,
                TEXT("%sUnable to disconnect from matchmaking lobby after we started following another host."),
                *this->GetLogPrefix(*Task));
            EMIT_TASK_COUNTER_TRACE(Task);
        }
    }
}

void FMatchmakingAlgorithmStepIncomingFollowRequestAccept::OnFollowResponseSent(
    const FOnlineError &Error,
    const FUniqueNetId &UserId,
    TWeakPtr<FMatchmakingEngineTask> TaskWk,
    TSharedPtr<const FOnlineLobbyId> TargetLobbyId,
    TSharedRef<const FUniqueNetId> TargetMemberId)
{
    auto Task = TaskWk.Pin();
    if (!Task.IsValid())
    {
        return;
    }

    if (!Task->CurrentStep.IsEqual(MMSTEP_INCOMING_FOLLOW_REQUEST_ACCEPT))
    {
        return;
    }

    EMIT_TASK_COUNTER_TRACE(Task);

    if (!Error.bSucceeded)
    {
        if (Error.ErrorCode.EndsWith(".timed_out"))
        {
            // We also go back to searching if we timeout. That's because follow requests have a very
            // short response period, and it's possible that we've hit not only the soft threshold
            // for response, but also the hard threshold for the requester to timeout even if we're
            // connected to their lobby.
        }

        // TODO: We need to call DisconnectLobby on the TargetLobbyId, because we're no longer joining it.
        // Right now the requester will kick us when we timeout anyway, but we really should clean up properly.

        // We couldn't tell the target that we're accepting their request, go back to searching.
        this->MoveToStep(*Task, MMSTEP_SEARCHING);
        return;
    }

    if (auto Pins = Task->PinAndContinue())
    {
        auto Txn = Pins->OnlineLobby->MakeUpdateLobbyTransaction(*Task->Request.UserId, *Task->MatchmakingLobbyId);
        Txn->Locked = true;
        Txn->SetMetadata.Add(MatchmakingMetadataLobbyStatus, MatchmakingStatusFollowing);
        Txn->SetMetadata.Add(MatchmakingMetadataLobbyHeartbeatTimestamp, FDateTime::UtcNow().ToUnixTimestamp());
        Txn->SetMetadata.Add(MatchmakingMetadataFollowTargetLobbyId, TargetLobbyId->ToString());

        EMIT_TASK_COUNTER_TRACE(Task);

        if (!Pins->OnlineLobby->UpdateLobby(
                *Task->Request.UserId,
                *Task->MatchmakingLobbyId,
                *Txn,
                FOnLobbyOperationComplete::CreateSP(
                    this,
                    &FMatchmakingAlgorithmStepIncomingFollowRequestAccept::OnMatchmakingLobbyUpdated,
                    TWeakPtr<FMatchmakingEngineTask>(Task),
                    TargetLobbyId,
                    MoveTemp(TargetMemberId))))
        {
            UE_LOG(
                LogMatchmaking,
                Error,
                TEXT("%sUnable to notify other clients they should join the target lobby, but we have already accepted "
                     "the follow request! Expect things to break."),
                *this->GetLogPrefix(*Task));
        }

        EMIT_TASK_COUNTER_TRACE(Task);
    }
}

void FMatchmakingAlgorithmStepIncomingFollowRequestAccept::Start(
    TSharedRef<FMatchmakingEngineTask> Task,
    TSharedPtr<const FOnlineLobbyId> TargetLobbyId,
    TSharedRef<const FUniqueNetId> TargetMemberId,
    int32 TargetRequestId)
{
    EMIT_TASK_COUNTER_TRACE(Task);

    if (auto Pins = Task->PinAndContinue())
    {
        auto Txn = Pins->OnlineLobby->MakeUpdateLobbyTransaction(
            *Task->Request.UserId,
            *Task->MatchmakingFollowRequestLobbyId);
        Txn->SetMetadata.Add(
            FString::Printf(TEXT("%s.%s"), MatchmakingMemberMetadataFollowResponse, *TargetMemberId->ToString()),
            FMatchmakingFollowResponse::SerializeFollowResponse(TargetRequestId, true));

        EMIT_TASK_COUNTER_TRACE(Task);

        if (!Pins->OnlineLobby->UpdateLobby(
                *Task->Request.UserId,
                *Task->MatchmakingFollowRequestLobbyId,
                *Txn,
                FOnLobbyOperationComplete::CreateSP(
                    this,
                    &FMatchmakingAlgorithmStepIncomingFollowRequestAccept::OnFollowResponseSent,
                    TWeakPtr<FMatchmakingEngineTask>(Task),
                    TargetLobbyId,
                    TargetMemberId)))
        {
            // We couldn't tell the remote client that we're joining their lobby,
            // so just fallback to searching and we'll timeout on the other side.

            // TODO: We need to call DisconnectLobby on the TargetLobbyId, because we're no longer joining it.
            // Right now the requester will kick us when we timeout anyway, but we really should clean up properly.

            EMIT_TASK_COUNTER_TRACE(Task);
            this->MoveToStep(*Task, MMSTEP_SEARCHING);
        }

        EMIT_TASK_COUNTER_TRACE(Task);
    }
}

#endif

REDPOINT_EOS_CODE_GUARD_END()