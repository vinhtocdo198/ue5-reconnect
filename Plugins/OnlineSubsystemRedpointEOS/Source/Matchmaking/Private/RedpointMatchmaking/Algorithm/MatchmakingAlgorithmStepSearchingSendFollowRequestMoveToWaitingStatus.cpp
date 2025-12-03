// Copyright June Rhodes. All Rights Reserved.

#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepSearchingSendFollowRequestMoveToWaitingStatus.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepSearchingSendFollowRequestDoRequest.h"
#include "RedpointMatchmaking/MatchmakingConstants.h"
#include "RedpointMatchmaking/MatchmakingLog.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

TSharedPtr<FMatchmakingAlgorithmStepStateSearchingSendFollowRequestMoveToWaitingStatus>
FMatchmakingAlgorithmStepSearchingSendFollowRequestMoveToWaitingStatus::CreateNewState()
{
    return MakeShared<FMatchmakingAlgorithmStepStateSearchingSendFollowRequestMoveToWaitingStatus>();
}

void FMatchmakingAlgorithmStepSearchingSendFollowRequestMoveToWaitingStatus::OnLobbyUpdated(
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

    if (!Task->CurrentStep.IsEqual(MMSTEP_SEARCHING_SEND_FOLLOW_REQUEST_MOVE_TO_WAITING_STATUS))
    {
        return;
    }

    if (!Error.bSucceeded)
    {
        if (Error.ErrorCode.EndsWith(".timed_out") || Error.ErrorCode.EndsWith(".already_pending"))
        {
            // Retry on timeout. It's safe to just call Start() again for this implementation.
            this->Start(Task.ToSharedRef(), MatchCandidate);
            return;
        }

        this->RestartableError(
            *Task,
            FString::Printf(TEXT("Unable to move matchmaking lobby to waiting status: %s"), *Error.ToLogString()));
        return;
    }

    if (auto Pins = Task->PinAndContinue())
    {
        // Track the users that are about to join, since they might start joining before we
        // get the actual accept message.
        verifyf(
            Task->TeamManager->TryAddPendingBatch(MatchCandidate.TeamReassignments, MatchCandidate.MemberSkillRating),
            TEXT("TryAddPendingBatch must succeed when it's passed TeamReassignments that we should have already "
                 "validated!"));

        EMIT_TASK_COUNTER_TRACE(Task);
        this->MoveToStep(*Task, MMSTEP_SEARCHING_SEND_FOLLOW_REQUEST_DO_REQUEST, MatchCandidate);
    }
}

void FMatchmakingAlgorithmStepSearchingSendFollowRequestMoveToWaitingStatus::Start(
    TSharedRef<FMatchmakingEngineTask> Task,
    FMatchmakingSuitableMatchResult MatchCandidate)
{
    if (auto Pins = Task->PinAndContinue())
    {
        UE_LOG(
            LogMatchmaking,
            Verbose,
            TEXT("%sMoving the matchmaking lobby %s to waiting status"),
            *this->GetLogPrefix(*Task),
            *Task->MatchmakingLobbyId->ToString());

        auto Transaction =
            Pins->OnlineLobby->MakeUpdateLobbyTransaction(*Task->Request.UserId, *Task->MatchmakingLobbyId);
        Transaction->SetMetadata.Add(MatchmakingMetadataLobbyStatus, MatchmakingStatusWaiting);
        Transaction->SetMetadata.Add(MatchmakingMetadataLobbyHeartbeatTimestamp, FDateTime::UtcNow().ToUnixTimestamp());

        if (!Pins->OnlineLobby->UpdateLobby(
                *Task->Request.UserId,
                *Task->MatchmakingLobbyId,
                *Transaction,
                FOnLobbyOperationComplete::CreateSP(
                    this,
                    &FMatchmakingAlgorithmStepSearchingSendFollowRequestMoveToWaitingStatus::OnLobbyUpdated,
                    TWeakPtr<FMatchmakingEngineTask>(Task),
                    MatchCandidate)))
        {
            this->RestartableError(
                *Task,
                TEXT("Unable to move matchmaking lobby to waiting status: UpdateLobby call failed"));
            return;
        }
    }
}

#endif

REDPOINT_EOS_CODE_GUARD_END()