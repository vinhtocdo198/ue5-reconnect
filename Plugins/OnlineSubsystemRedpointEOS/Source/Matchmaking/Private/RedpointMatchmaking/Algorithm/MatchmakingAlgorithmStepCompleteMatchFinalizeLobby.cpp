// Copyright June Rhodes. All Rights Reserved.

#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepCompleteMatchFinalizeLobby.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepAbandonStart.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepCompleteMatchAsHost.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepCompleteMatchComputeResult.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepUnlockParty.h"
#include "RedpointMatchmaking/EventHandlers/MatchmakingEventHandlerFinalization.h"
#include "RedpointMatchmaking/MatchmakingConstants.h"
#include "RedpointMatchmaking/MatchmakingLog.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

TSharedPtr<FMatchmakingAlgorithmStepStateCompleteMatchFinalizeLobby>
FMatchmakingAlgorithmStepCompleteMatchFinalizeLobby::CreateNewState()
{
    auto State = MakeShared<FMatchmakingAlgorithmStepStateCompleteMatchFinalizeLobby>();
    State->bHasFinishedLobbyUpdate = false;
    State->OnTimeoutHandle.Reset();
    return State;
}

void FMatchmakingAlgorithmStepCompleteMatchFinalizeLobby::CheckLobbyFinalized(
    const TSharedRef<FMatchmakingEngineTask> &Task)
{
    auto State = this->GetState(*Task);
    if (!State->bHasFinishedLobbyUpdate)
    {
        // Until OnLobbyUpdated runs, we can't handle a CheckLobbyFinalized call from an interrupt. This prevents a race
        // condition where calling UpdateLobby causes the interrupt to fire before OnLobbyUpdated runs. When
        // OnLobbyUpdated runs, it would then call CheckLobbyFinalized, which would pass and try to call MoveToStep
        // again.
        return;
    }

    if (auto Pins = Task->PinAndContinue())
    {
        int32 MemberCount;
        if (!Pins->OnlineLobby->GetMemberCount(*Task->Request.UserId, *Task->MatchmakingLobbyId, MemberCount))
        {
            this->RestartableError(*Task, TEXT("Unable to get member count from matchmaking lobby!"));
            return;
        }

        for (int32 i = 0; i < MemberCount; i++)
        {
            TSharedPtr<const FUniqueNetId> MemberId;
            if (!Pins->OnlineLobby->GetMemberUserId(*Task->Request.UserId, *Task->MatchmakingLobbyId, i, MemberId))
            {
                this->RestartableError(
                    *Task,
                    FString::Printf(TEXT("Unable to get member at index %d from matchmaking lobby!"), i));
                return;
            }

            if (*MemberId == *Task->Request.UserId)
            {
                // Don't need to check the host for finalization.
                continue;
            }

            FVariantData FinalizedData;
            if (!Pins->OnlineLobby->GetMemberMetadataValue(
                    *Task->Request.UserId,
                    *Task->MatchmakingLobbyId,
                    *MemberId,
                    MatchmakingMemberMetadataFinalized,
                    FinalizedData))
            {
                // This member isn't finalized, we're not ready yet.
                return;
            }

            if (FinalizedData.GetType() != EOnlineKeyValuePairDataType::Bool)
            {
                // This member isn't finalized, we're not ready yet.
                return;
            }

            bool bIsFinalized;
            FinalizedData.GetValue(bIsFinalized);
            if (!bIsFinalized)
            {
                // This member isn't finalized, we're not ready yet.
                return;
            }
        }

        // If we haven't returned at this point, then the lobby is finalized and the host is ready to finish.

        if (State->OnTimeoutHandle.IsValid())
        {
            Task->Unschedule.Execute(State->OnTimeoutHandle);
            State->OnTimeoutHandle.Reset();
        }

        EMIT_TASK_COUNTER_TRACE(Task);
        this->MoveToStep(*Task, MMSTEP_COMPLETE_UNLOCK_PARTY);
    }
}

void FMatchmakingAlgorithmStepCompleteMatchFinalizeLobby::OnTimeout(
    TWeakPtr<FMatchmakingEngineTask> TaskWk)
{
    // We ran out of time waiting for players. Restart matchmaking.
    if (auto Task = TaskWk.Pin())
    {
        if (Task.IsValid() && Task->CurrentStep.IsEqual(MMSTEP_COMPLETE_MATCH_FINALIZE_LOBBY))
        {
            UE_LOG(
                LogMatchmaking,
                Verbose,
                TEXT("%sTimed out while finalizing matchmaking lobby %s, restarting matchmaking..."),
                *this->GetLogPrefix(*Task),
                *Task->MatchmakingLobbyId->ToString());

            this->MoveToStep(*Task, MMSTEP_ABANDON_START, EMatchmakingAbandonmentType::Restart);
        }
    }
}

void FMatchmakingAlgorithmStepCompleteMatchFinalizeLobby::OnLobbyUpdated(
    const FOnlineError &Error,
    const FUniqueNetId &UserId,
    TWeakPtr<FMatchmakingEngineTask> TaskWk)
{
    auto Task = TaskWk.Pin();
    if (!Task.IsValid())
    {
        return;
    }

    if (!Task->CurrentStep.IsEqual(MMSTEP_COMPLETE_MATCH_FINALIZE_LOBBY))
    {
        return;
    }

    if (!Error.bSucceeded)
    {
        if (Error.ErrorCode.EndsWith(".timed_out") || Error.ErrorCode.EndsWith(".already_pending"))
        {
            // Retry on timeout.
            if (auto Pins = Task->PinAndContinue())
            {
                auto Txn =
                    Pins->OnlineLobby->MakeUpdateLobbyTransaction(*Task->Request.UserId, *Task->MatchmakingLobbyId);
                Txn->SetMetadata.Add(MatchmakingMetadataLobbyStatus, MatchmakingStatusFinalizing);
                Txn->SetMetadata.Add(MatchmakingMetadataLobbyHeartbeatTimestamp, FDateTime::UtcNow().ToUnixTimestamp());
                if (!Pins->OnlineLobby->UpdateLobby(
                        *Task->Request.UserId,
                        *Task->MatchmakingLobbyId,
                        *Txn,
                        FOnLobbyOperationComplete::CreateSP(
                            this,
                            &FMatchmakingAlgorithmStepCompleteMatchFinalizeLobby::OnLobbyUpdated,
                            TWeakPtr<FMatchmakingEngineTask>(Task))))
                {
                    this->RestartableError(*Task, TEXT("Unable to finalize matchmaking lobby on retry for timed_out"));
                }
                return;
            }
        }

        this->RestartableError(
            *Task,
            FString::Printf(TEXT("Unable to finalize matchmaking lobby: %s"), *Error.ToLogString()));
        return;
    }

    auto State = this->GetState(*Task);
    State->bHasFinishedLobbyUpdate = true;

    State->OnTimeoutHandle = Task->Schedule.Execute(
        MatchmakingTimingWaitForPlayersTimeout,
        FMatchmakingEngineTaskScheduleCallback::CreateSP(
            this,
            &FMatchmakingAlgorithmStepCompleteMatchFinalizeLobby::OnTimeout,
            TWeakPtr<FMatchmakingEngineTask>(Task)));

    this->CheckLobbyFinalized(Task.ToSharedRef());
}

void FMatchmakingAlgorithmStepCompleteMatchFinalizeLobby::Start(TSharedRef<FMatchmakingEngineTask> Task)
{
    if (auto Pins = Task->PinAndContinue())
    {
        UE_LOG(
            LogMatchmaking,
            Verbose,
            TEXT("%sFinalizing matchmaking lobby %s"),
            *this->GetLogPrefix(*Task),
            *Task->MatchmakingLobbyId->ToString());

        auto Txn = Pins->OnlineLobby->MakeUpdateLobbyTransaction(*Task->Request.UserId, *Task->MatchmakingLobbyId);
        Txn->SetMetadata.Add(MatchmakingMetadataLobbyStatus, MatchmakingStatusFinalizing);
        Txn->SetMetadata.Add(MatchmakingMetadataLobbyHeartbeatTimestamp, FDateTime::UtcNow().ToUnixTimestamp());

        auto State = this->GetState(*Task);
        State->bHasFinishedLobbyUpdate = false;
        State->OnTimeoutHandle.Reset();

        if (!Pins->OnlineLobby->UpdateLobby(
                *Task->Request.UserId,
                *Task->MatchmakingLobbyId,
                *Txn,
                FOnLobbyOperationComplete::CreateSP(
                    this,
                    &FMatchmakingAlgorithmStepCompleteMatchFinalizeLobby::OnLobbyUpdated,
                    TWeakPtr<FMatchmakingEngineTask>(Task))))
        {
            this->RestartableError(*Task, TEXT("Unable to finalize matchmaking lobby"));
            return;
        }
    }
}

bool FMatchmakingAlgorithmStepCompleteMatchFinalizeLobby::CanHandleInterrupt(
    const TSharedRef<FMatchmakingEngineTask> &Task,
    FName InterruptType)
{
    return InterruptType.IsEqual(MMINTERRUPT_CLIENT_FINALIZED_RESULT);
}

bool FMatchmakingAlgorithmStepCompleteMatchFinalizeLobby::HandleInterrupt(
    const TSharedRef<FMatchmakingEngineTask> &Task,
    FName InterruptType)
{
    if (InterruptType.IsEqual(MMINTERRUPT_CLIENT_FINALIZED_RESULT))
    {
        this->CheckLobbyFinalized(Task);
        return true;
    }

    return false;
}

#endif

REDPOINT_EOS_CODE_GUARD_END()