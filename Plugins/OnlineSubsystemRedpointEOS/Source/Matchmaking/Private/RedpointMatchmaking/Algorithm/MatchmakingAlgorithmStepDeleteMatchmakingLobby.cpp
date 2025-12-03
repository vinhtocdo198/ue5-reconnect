// Copyright June Rhodes. All Rights Reserved.

#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepDeleteMatchmakingLobby.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/MatchmakingConstants.h"
#include "RedpointMatchmaking/MatchmakingLog.h"
#include "RedpointMatchmaking/MatchmakingSerialization.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

TSharedPtr<FMatchmakingAlgorithmStepStateDeleteMatchmakingLobby> FMatchmakingAlgorithmStepDeleteMatchmakingLobby::
    CreateNewState()
{
    return MakeShared<FMatchmakingAlgorithmStepStateDeleteMatchmakingLobby>();
}

void FMatchmakingAlgorithmStepDeleteMatchmakingLobby::OnLobbyDeleted(
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

    if (!Task->CurrentStep.IsEqual(MMSTEP_DELETE_MATCHMAKING_LOBBY))
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
            TEXT("%sFailed to delete matchmaking lobby %s: %s"),
            *this->GetLogPrefix(*Task),
            *Task->MatchmakingLobbyId->ToString(),
            *Error.ToLogString());
    }

    Task->MatchmakingLobbyId.Reset();
    EMIT_TASK_COUNTER_TRACE(Task);
    this->MoveToStep(*Task, NextStep);
}

void FMatchmakingAlgorithmStepDeleteMatchmakingLobby::Start(TSharedRef<FMatchmakingEngineTask> Task, FName NextStep)
{
    if (auto Pins = Task->PinAndContinue())
    {
        UE_LOG(
            LogMatchmaking,
            Verbose,
            TEXT("%sDeleting the matchmaking follow request lobby"),
            *this->GetLogPrefix(*Task));

        // We're about to delete the lobby, so we must store the response for CompleteMatchAsHost to use later.
        auto Result = MakeShared<FMatchmakingEngineResponse>();
        Result->Teams = FMatchmakingSerialization::DeserializeResultFromLobbyData(
            Pins->OnlineLobby.ToSharedRef(),
            Pins->OnlineIdentity.ToSharedRef(),
            *Task->Request.UserId,
            *Task->MatchmakingLobbyId,
            MatchmakingMetadataResult);
        Task->StoredResponse = Result;

        if (!Pins->OnlineLobby->DeleteLobby(
                *Task->Request.UserId,
                *Task->MatchmakingLobbyId,
                FOnLobbyOperationComplete::CreateSP(
                    this,
                    &FMatchmakingAlgorithmStepDeleteMatchmakingLobby::OnLobbyDeleted,
                    TWeakPtr<FMatchmakingEngineTask>(Task),
                    NextStep)))
        {
            UE_LOG(
                LogMatchmaking,
                Error,
                TEXT("%sFailed to delete matchmaking follow request lobby %s"),
                *this->GetLogPrefix(*Task),
                *Task->MatchmakingLobbyId->ToString());
            Task->MatchmakingLobbyId.Reset();
            EMIT_TASK_COUNTER_TRACE(Task);
            this->MoveToStep(*Task, NextStep);
            return;
        }
    }
}

#endif

REDPOINT_EOS_CODE_GUARD_END()