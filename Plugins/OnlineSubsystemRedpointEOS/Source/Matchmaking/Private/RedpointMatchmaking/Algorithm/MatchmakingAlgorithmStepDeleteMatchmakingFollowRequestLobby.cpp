// Copyright June Rhodes. All Rights Reserved.

#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepDeleteMatchmakingFollowRequestLobby.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/MatchmakingLog.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

TSharedPtr<FMatchmakingAlgorithmStepStateDeleteMatchmakingFollowRequestLobby>
FMatchmakingAlgorithmStepDeleteMatchmakingFollowRequestLobby::CreateNewState()
{
    return MakeShared<FMatchmakingAlgorithmStepStateDeleteMatchmakingFollowRequestLobby>();
}

void FMatchmakingAlgorithmStepDeleteMatchmakingFollowRequestLobby::OnLobbyDeleted(
    const FOnlineError &Error,
    const FUniqueNetId &UserId,
    TWeakPtr<FMatchmakingEngineTask> TaskWk,
    FName NextStep)
{
    auto Task = TaskWk.Pin();
    if (!Task.IsValid())
    {
        return;
    }

    if (!Task->CurrentStep.IsEqual(MMSTEP_DELETE_MATCHMAKING_FOLLOW_REQUEST_LOBBY))
    {
        return;
    }

    if (!Error.bSucceeded)
    {
        if (Error.ErrorCode.EndsWith(".timed_out") || Error.ErrorCode.EndsWith(".already_pending"))
        {
            // Retry on timeout. It's safe to just call Start() again for this implementation.
            this->Start(Task.ToSharedRef(), NextStep);
            return;
        }

        UE_LOG(
            LogMatchmaking,
            Error,
            TEXT("%sFailed to delete matchmaking follow request lobby %s: %s"),
            *this->GetLogPrefix(*Task),
            *Task->MatchmakingFollowRequestLobbyId->ToString(),
            *Error.ToLogString());
    }

    Task->MatchmakingFollowRequestLobbyId.Reset();
    EMIT_TASK_COUNTER_TRACE(Task);
    this->MoveToStep(*Task, NextStep);
}

void FMatchmakingAlgorithmStepDeleteMatchmakingFollowRequestLobby::Start(
    TSharedRef<FMatchmakingEngineTask> Task,
    FName NextStep)
{
    if (auto Pins = Task->PinAndContinue())
    {
        UE_LOG(
            LogMatchmaking,
            Verbose,
            TEXT("%sDeleting the matchmaking follow request lobby"),
            *this->GetLogPrefix(*Task));

        if (!Pins->OnlineLobby->DeleteLobby(
                *Task->Request.UserId,
                *Task->MatchmakingFollowRequestLobbyId,
                FOnLobbyOperationComplete::CreateSP(
                    this,
                    &FMatchmakingAlgorithmStepDeleteMatchmakingFollowRequestLobby::OnLobbyDeleted,
                    TWeakPtr<FMatchmakingEngineTask>(Task),
                    NextStep)))
        {
            UE_LOG(
                LogMatchmaking,
                Error,
                TEXT("%sFailed to delete matchmaking follow request lobby %s"),
                *this->GetLogPrefix(*Task),
                *Task->MatchmakingFollowRequestLobbyId->ToString());
            Task->MatchmakingFollowRequestLobbyId.Reset();
            EMIT_TASK_COUNTER_TRACE(Task);
            this->MoveToStep(*Task, NextStep);
            return;
        }
    }
}

#endif

REDPOINT_EOS_CODE_GUARD_END()