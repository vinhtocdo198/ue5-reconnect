// Copyright June Rhodes. All Rights Reserved.

#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepSearchingMoveToSearchingStatus.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepCompleteMatch.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepSearchingPerformSearch.h"
#include "RedpointMatchmaking/EventHandlers/MatchmakingEventHandlerReceiveFollowRequest.h"
#include "RedpointMatchmaking/MatchmakingConstants.h"
#include "RedpointMatchmaking/MatchmakingLog.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

TSharedPtr<FMatchmakingAlgorithmStepStateSearchingMoveToSearchingStatus>
FMatchmakingAlgorithmStepSearchingMoveToSearchingStatus::CreateNewState()
{
    return MakeShared<FMatchmakingAlgorithmStepStateSearchingMoveToSearchingStatus>();
}

void FMatchmakingAlgorithmStepSearchingMoveToSearchingStatus::OnLobbyUpdated(
    const FOnlineError &Error,
    const FUniqueNetId &UserId,
    TWeakPtr<FMatchmakingEngineTask> TaskWk)
{
    auto Task = TaskWk.Pin();
    if (!Task.IsValid())
    {
        return;
    }

    auto State = this->GetState(*Task);
    if (State->bInterruptedWithFollowRequest)
    {
        // We are no longer the current step.
        return;
    }

    if (!Task->CurrentStep.IsEqual(MMSTEP_SEARCHING_MOVE_TO_SEARCHING_STATUS))
    {
        return;
    }

    if (!Error.bSucceeded)
    {
        if (Error.ErrorCode.EndsWith(".timed_out") || Error.ErrorCode.EndsWith(".already_pending"))
        {
            if (auto Pins = Task->PinAndContinue())
            {
                auto Transaction =
                    Pins->OnlineLobby->MakeUpdateLobbyTransaction(*Task->Request.UserId, *Task->MatchmakingLobbyId);
                Transaction->SetMetadata.Add(MatchmakingMetadataLobbyStatus, MatchmakingStatusSearching);
                Transaction->SetMetadata.Add(
                    MatchmakingMetadataLobbyRequestId,
                    Task->Request.HostConfiguration->RequestId);
                Transaction->SetMetadata.Add(
                    MatchmakingMetadataLobbyHeartbeatTimestamp,
                    FDateTime::UtcNow().ToUnixTimestamp());
                if (!Pins->OnlineLobby->UpdateLobby(
                        *Task->Request.UserId,
                        *Task->MatchmakingLobbyId,
                        *Transaction,
                        FOnLobbyOperationComplete::CreateSP(
                            this,
                            &FMatchmakingAlgorithmStepSearchingMoveToSearchingStatus::OnLobbyUpdated,
                            TWeakPtr<FMatchmakingEngineTask>(Task))))
                {
                    this->RestartableError(
                        *Task,
                        TEXT("Unable to move matchmaking lobby to searching status on retry for timed_out: "
                             "UpdateLobby call failed"));
                }
                return;
            }
        }

        this->RestartableError(
            *Task,
            FString::Printf(TEXT("Unable to move matchmaking lobby to searching status: %s"), *Error.ToLogString()));
        return;
    }

    EMIT_TASK_COUNTER_TRACE(Task);

    this->MoveToStep(*Task, MMSTEP_SEARCHING_PERFORM_SEARCH);
}

void FMatchmakingAlgorithmStepSearchingMoveToSearchingStatus::Start(TSharedRef<FMatchmakingEngineTask> Task)
{
    if (auto Pins = Task->PinAndContinue())
    {
        auto State = this->GetState(*Task);
        State->bInterruptedWithFollowRequest = false;

        // This is a safety check in case we get to this point and the match has been filled. Typically this is
        // checked after players have joined a lobby after a follow request is complete, but if you enter matchmaking
        // with a single 1 player slot, and the host is added, then you can end up in a loop because no results
        // from the search are suitable to complete the lobby (because you're already full). So this check ensures
        // that if we were ever about to perform a search, and we don't actually need to, that we skip straight to
        // completing the match.
        if (Task->TeamManager->IsMatchComplete())
        {
            // Match is full, complete it.
            UE_LOG(LogMatchmaking, Verbose, TEXT("%sMatch is now complete"), *this->GetLogPrefix(*Task));
            EMIT_TASK_COUNTER_TRACE(Task);
            this->MoveToStep(*Task, MMSTEP_COMPLETE_MATCH);
            return;
        }

        UE_LOG(
            LogMatchmaking,
            Verbose,
            TEXT("%sMoving the matchmaking lobby %s to searching status"),
            *this->GetLogPrefix(*Task),
            *Task->MatchmakingLobbyId->ToString());

        auto Transaction =
            Pins->OnlineLobby->MakeUpdateLobbyTransaction(*Task->Request.UserId, *Task->MatchmakingLobbyId);
        Transaction->SetMetadata.Add(MatchmakingMetadataLobbyStatus, MatchmakingStatusSearching);
        Transaction->SetMetadata.Add(MatchmakingMetadataLobbyRequestId, Task->Request.HostConfiguration->RequestId);
        Transaction->SetMetadata.Add(MatchmakingMetadataLobbyHeartbeatTimestamp, FDateTime::UtcNow().ToUnixTimestamp());
        if (!Task->Request.HostConfiguration->SkillStatPrefix.IsEmpty())
        {
            Transaction->SetMetadata.Add(
                MatchmakingMetadataLobbySkillOrdinal,
                Task->TeamManager->GetMatchAverageOpenskillOrdinal());
        }

        if (!Pins->OnlineLobby->UpdateLobby(
                *Task->Request.UserId,
                *Task->MatchmakingLobbyId,
                *Transaction,
                FOnLobbyOperationComplete::CreateSP(
                    this,
                    &FMatchmakingAlgorithmStepSearchingMoveToSearchingStatus::OnLobbyUpdated,
                    TWeakPtr<FMatchmakingEngineTask>(Task))))
        {
            this->RestartableError(
                *Task,
                TEXT("Unable to move matchmaking lobby to searching status: UpdateLobby call failed"));
            return;
        }
    }
}

bool FMatchmakingAlgorithmStepSearchingMoveToSearchingStatus::CanHandleInterrupt(
    const TSharedRef<FMatchmakingEngineTask> &Task,
    FName InterruptType)
{
    return InterruptType.IsEqual(MMINTERRUPT_WANT_TO_ACCEPT_FOLLOW_REQUEST);
}

bool FMatchmakingAlgorithmStepSearchingMoveToSearchingStatus::HandleInterrupt(
    const TSharedRef<FMatchmakingEngineTask> &Task,
    FName InterruptType)
{
    if (InterruptType.IsEqual(MMINTERRUPT_WANT_TO_ACCEPT_FOLLOW_REQUEST))
    {
        auto State = this->GetState(*Task);
        State->bInterruptedWithFollowRequest = true;
        UE_LOG(
            LogMatchmaking,
            Verbose,
            TEXT("%sMove to searching status interrupted by follow request for lobby %s"),
            *this->GetLogPrefix(*Task),
            *Task->MatchmakingLobbyId->ToString());
        return true;
    }

    return false;
}

#endif

REDPOINT_EOS_CODE_GUARD_END()