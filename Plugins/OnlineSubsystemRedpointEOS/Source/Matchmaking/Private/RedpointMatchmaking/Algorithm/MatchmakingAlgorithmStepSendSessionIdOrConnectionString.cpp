// Copyright June Rhodes. All Rights Reserved.

#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepSendSessionIdOrConnectionString.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepCompleteMatchFireEvent.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepDeleteMatchmakingLobby.h"
#include "RedpointMatchmaking/EventHandlers/MatchmakingEventHandlerFinalization.h"
#include "RedpointMatchmaking/MatchmakingConstants.h"
#include "RedpointMatchmaking/MatchmakingErrors.h"
#include "RedpointMatchmaking/MatchmakingLog.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

TSharedPtr<FMatchmakingAlgorithmStepStateSendSessionIdOrConnectionString>
FMatchmakingAlgorithmStepSendSessionIdOrConnectionString::CreateNewState()
{
    return MakeShared<FMatchmakingAlgorithmStepStateSendSessionIdOrConnectionString>();
}

void FMatchmakingAlgorithmStepSendSessionIdOrConnectionString::CheckLobbyConnecting(
    const TSharedRef<FMatchmakingEngineTask> &Task)
{
    auto State = this->GetState(*Task);
    if (!State->bHasFinishedLobbyUpdate)
    {
        // Until OnLobbyUpdated runs, we can't handle a CheckLobbyConnecting call from an interrupt. This prevents a
        // race condition where calling UpdateLobby causes the interrupt to fire before OnLobbyUpdated runs. When
        // OnLobbyUpdated runs, it would then call CheckLobbyConnecting, which would pass and try to call MoveToStep
        // again.
        return;
    }

    if (auto Pins = Task->PinAndContinue())
    {
        int32 MemberCount;
        if (!Pins->OnlineLobby->GetMemberCount(*Task->Request.UserId, *Task->MatchmakingLobbyId, MemberCount))
        {
            UE_LOG(
                LogMatchmaking,
                Error,
                TEXT("%sUnable to get member count from matchmaking lobby!"),
                *this->GetLogPrefix(*Task));
            this->PermanentError(*Task, OnlineMatchmaking::Errors::UnableToUpdateLobbyWithConnectionString());
            return;
        }

        for (int32 i = 0; i < MemberCount; i++)
        {
            TSharedPtr<const FUniqueNetId> MemberId;
            if (!Pins->OnlineLobby->GetMemberUserId(*Task->Request.UserId, *Task->MatchmakingLobbyId, i, MemberId))
            {
                UE_LOG(
                    LogMatchmaking,
                    Error,
                    TEXT("%sUnable to get member at index %d from matchmaking lobby!"),
                    *this->GetLogPrefix(*Task),
                    i);
                this->PermanentError(*Task, OnlineMatchmaking::Errors::UnableToUpdateLobbyWithConnectionString());
                return;
            }

            if (*MemberId == *Task->Request.UserId)
            {
                // Don't need to check the host for finalization.
                continue;
            }

            FVariantData ConnectingData;
            if (!Pins->OnlineLobby->GetMemberMetadataValue(
                    *Task->Request.UserId,
                    *Task->MatchmakingLobbyId,
                    *MemberId,
                    MatchmakingMemberMetadataConnecting,
                    ConnectingData))
            {
                // This member isn't connecting, we're not ready yet.
                return;
            }

            if (ConnectingData.GetType() != EOnlineKeyValuePairDataType::Bool)
            {
                // This member isn't connecting, we're not ready yet.
                return;
            }

            bool bIsConnecting;
            ConnectingData.GetValue(bIsConnecting);
            if (!bIsConnecting)
            {
                // This member isn't connecting, we're not ready yet.
                return;
            }
        }

        // If we haven't returned at this point, then the lobby is connecting and the host is ready to finish.

        if (State->OnTimeoutHandle.IsValid())
        {
            Task->Unschedule.Execute(State->OnTimeoutHandle);
            State->OnTimeoutHandle.Reset();
        }

        EMIT_TASK_COUNTER_TRACE(Task);
        this->MoveToStep(*Task, State->NextStep);
    }
}

void FMatchmakingAlgorithmStepSendSessionIdOrConnectionString::OnTimeout(TWeakPtr<FMatchmakingEngineTask> TaskWk)
{
    // We ran out of time waiting for players.
    if (auto Task = TaskWk.Pin())
    {
        if (Task.IsValid() && Task->CurrentStep.IsEqual(MMSTEP_SEND_SESSION_ID_OR_CONNECTION_STRING))
        {
            UE_LOG(
                LogMatchmaking,
                Error,
                TEXT("%sTimed out while waiting for other players to receive connection information."),
                *this->GetLogPrefix(*Task));
            this->PermanentError(*Task, OnlineMatchmaking::Errors::UnableToUpdateLobbyWithConnectionString());
        }
    }
}

void FMatchmakingAlgorithmStepSendSessionIdOrConnectionString::OnLobbyUpdated(
    const FOnlineError &Error,
    const FUniqueNetId &UserId,
    TWeakPtr<FMatchmakingEngineTask> TaskWk)
{
    auto Task = TaskWk.Pin();
    if (!Task.IsValid())
    {
        return;
    }

    if (!Task->CurrentStep.IsEqual(MMSTEP_SEND_SESSION_ID_OR_CONNECTION_STRING))
    {
        return;
    }

    if (!Error.bSucceeded)
    {
        if (Error.ErrorCode.EndsWith(".timed_out") || Error.ErrorCode.EndsWith(".already_pending"))
        {
            this->StartLobbyUpdate(Task.ToSharedRef());
            return;
        }

        UE_LOG(
            LogMatchmaking,
            Error,
            TEXT("%sUnable to send connection information to other match members"),
            *this->GetLogPrefix(*Task));
        this->PermanentError(*Task, OnlineMatchmaking::Errors::UnableToUpdateLobbyWithConnectionString());
        return;
    }

    auto State = this->GetState(*Task);
    State->bHasFinishedLobbyUpdate = true;

    State->OnTimeoutHandle = Task->Schedule.Execute(
        MatchmakingTimingWaitForPlayersTimeout,
        FMatchmakingEngineTaskScheduleCallback::CreateSP(
            this,
            &FMatchmakingAlgorithmStepSendSessionIdOrConnectionString::OnTimeout,
            TWeakPtr<FMatchmakingEngineTask>(Task)));

    this->CheckLobbyConnecting(Task.ToSharedRef());
}

void FMatchmakingAlgorithmStepSendSessionIdOrConnectionString::Start(
    TSharedRef<FMatchmakingEngineTask> Task,
    FName NextStep,
    FString ConnectionKey,
    FString ConnectionValue)
{
    auto State = this->GetState(*Task);
    State->NextStep = NextStep;
    State->ConnectionKey = ConnectionKey;
    State->ConnectionValue = ConnectionValue;

    this->StartLobbyUpdate(Task);
}

bool FMatchmakingAlgorithmStepSendSessionIdOrConnectionString::CanHandleInterrupt(
    const TSharedRef<FMatchmakingEngineTask> &Task,
    FName InterruptType)
{
    return InterruptType.IsEqual(MMINTERRUPT_CLIENT_CONNECTING_STARTED);
}

bool FMatchmakingAlgorithmStepSendSessionIdOrConnectionString::HandleInterrupt(
    const TSharedRef<FMatchmakingEngineTask> &Task,
    FName InterruptType)
{
    if (InterruptType.IsEqual(MMINTERRUPT_CLIENT_CONNECTING_STARTED))
    {
        this->CheckLobbyConnecting(Task);
        return true;
    }

    return false;
}

void FMatchmakingAlgorithmStepSendSessionIdOrConnectionString::StartLobbyUpdate(TSharedRef<FMatchmakingEngineTask> Task)
{
    auto State = this->GetState(*Task);

    if (auto Pins = Task->PinAndContinue())
    {
        auto Txn = Pins->OnlineLobby->MakeUpdateLobbyTransaction(*Task->Request.UserId, *Task->MatchmakingLobbyId);
        if (State->ConnectionKey == MatchmakingMetadataConnectionSessionId)
        {
            UE_LOG(
                LogMatchmaking,
                Verbose,
                TEXT("%sSending session ID '%s' to other match members"),
                *this->GetLogPrefix(*Task),
                *State->ConnectionValue);
        }
        else
        {
            UE_LOG(
                LogMatchmaking,
                Verbose,
                TEXT("%sSending connection URL '%s' to other match members"),
                *this->GetLogPrefix(*Task),
                *State->ConnectionValue);
        }
        Txn->SetMetadata.Add(State->ConnectionKey, State->ConnectionValue);

        State->bHasFinishedLobbyUpdate = false;
        State->OnTimeoutHandle.Reset();

        if (!Pins->OnlineLobby->UpdateLobby(
                *Task->Request.UserId,
                *Task->MatchmakingLobbyId,
                *Txn,
                FOnLobbyOperationComplete::CreateSP(
                    this,
                    &FMatchmakingAlgorithmStepSendSessionIdOrConnectionString::OnLobbyUpdated,
                    TWeakPtr<FMatchmakingEngineTask>(Task))))
        {
            UE_LOG(
                LogMatchmaking,
                Error,
                TEXT("%sUnable to send connection information to other match members"),
                *this->GetLogPrefix(*Task));
            this->PermanentError(*Task, OnlineMatchmaking::Errors::UnableToUpdateLobbyWithConnectionString());
            return;
        }
    }
}

#endif

REDPOINT_EOS_CODE_GUARD_END()