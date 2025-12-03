// Copyright June Rhodes. All Rights Reserved.

#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepWaitForSessionIdOrConnectionString.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "../MatchmakingSerialization.h"
#include "./MatchmakingAlgorithmStepConnectToGameServer.h"
#include "./MatchmakingAlgorithmStepJoinServerSession.h"
#include "RedpointMatchmaking/EventHandlers/MatchmakingEventHandlerConnectionInfoProvided.h"
#include "RedpointMatchmaking/MatchmakingConstants.h"
#include "RedpointMatchmaking/MatchmakingLog.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

TSharedPtr<FMatchmakingAlgorithmStepStateWaitForSessionIdOrConnectionString>
FMatchmakingAlgorithmStepWaitForSessionIdOrConnectionString::CreateNewState()
{
    return MakeShared<FMatchmakingAlgorithmStepStateWaitForSessionIdOrConnectionString>();
}

void FMatchmakingAlgorithmStepWaitForSessionIdOrConnectionString::Start(TSharedRef<FMatchmakingEngineTask> Task)
{
}

bool FMatchmakingAlgorithmStepWaitForSessionIdOrConnectionString::CanHandleInterrupt(
    const TSharedRef<FMatchmakingEngineTask> &Task,
    FName InterruptType)
{
    return InterruptType.IsEqual(MMINTERRUPT_RECEIVED_CONNECTION_SESSION_ID) ||
           InterruptType.IsEqual(MMINTERRUPT_RECEIVED_CONNECTION_URL);
}

bool FMatchmakingAlgorithmStepWaitForSessionIdOrConnectionString::HandleInterrupt(
    const TSharedRef<FMatchmakingEngineTask> &Task,
    FName InterruptType)
{
    EMIT_TASK_COUNTER_TRACE(Task);

    if (this->GetState(*Task)->bIsMarkingAsConnecting)
    {
        return false;
    }

    if (Task->MatchmakingLobbyId.IsValid())
    {
        if (auto Pins = Task->PinAndContinue())
        {
            FName NextStep;
            FString ConnectionUrlOrSessionId;
            bool bShouldMarkAsConnecting = false;

            FVariantData Data;
            if (Pins->OnlineLobby->GetLobbyMetadataValue(
                    *Task->Request.UserId,
                    *Task->MatchmakingLobbyId,
                    MatchmakingMetadataConnectionSessionId,
                    Data) &&
                Data.GetType() == EOnlineKeyValuePairDataType::String)
            {
                Data.GetValue(ConnectionUrlOrSessionId);
                NextStep = MMSTEP_JOIN_SERVER_SESSION;
                bShouldMarkAsConnecting = true;
            }
            else if (
                Pins->OnlineLobby->GetLobbyMetadataValue(
                    *Task->Request.UserId,
                    *Task->MatchmakingLobbyId,
                    MatchmakingMetadataConnectionUrl,
                    Data) &&
                Data.GetType() == EOnlineKeyValuePairDataType::String)
            {
                Data.GetValue(ConnectionUrlOrSessionId);
                NextStep = MMSTEP_CONNECT_TO_GAME_SERVER;
                bShouldMarkAsConnecting = true;
            }

            if (bShouldMarkAsConnecting)
            {
                this->GetState(*Task)->bIsMarkingAsConnecting = true;

                auto Txn = Pins->OnlineLobby->MakeUpdateLobbyMemberTransaction(
                    *Task->Request.UserId,
                    *Task->MatchmakingLobbyId,
                    *Task->Request.UserId);
                Txn->SetMetadata.Add(MatchmakingMemberMetadataConnecting, true);
                if (!Pins->OnlineLobby->UpdateMemberSelf(
                        *Task->Request.UserId,
                        *Task->MatchmakingLobbyId,
                        *Txn,
                        FOnLobbyOperationComplete::CreateSP(
                            this,
                            &FMatchmakingAlgorithmStepWaitForSessionIdOrConnectionString::OnMarkedAsConnecting,
                            TWeakPtr<FMatchmakingEngineTask>(Task),
                            NextStep,
                            ConnectionUrlOrSessionId)))
                {
                    this->GetState(*Task)->bIsMarkingAsConnecting = false;
                    this->RestartableError(*Task, TEXT("Unable to tell the host that we're connecting on the client!"));
                }

                return true;
            }
        }
    }

    return false;
}

void FMatchmakingAlgorithmStepWaitForSessionIdOrConnectionString::OnMarkedAsConnecting(
    const FOnlineError &Error,
    const FUniqueNetId &UserId,
    TWeakPtr<FMatchmakingEngineTask> TaskWk,
    FName NextStep,
    FString ConnectionUrlOrSessionId)
{
    auto Task = TaskWk.Pin();
    if (!Task.IsValid())
    {
        return;
    }

    if (!Task->CurrentStep.IsEqual(MMSTEP_WAIT_FOR_SESSION_ID_OR_CONNECTION_STRING))
    {
        return;
    }

    if (!Error.bSucceeded)
    {
        if (Error.ErrorCode.EndsWith(".timed_out") || Error.ErrorCode.EndsWith(".already_pending"))
        {
            if (auto Pins = Task->PinAndContinue())
            {
                auto Txn = Pins->OnlineLobby->MakeUpdateLobbyMemberTransaction(
                    *Task->Request.UserId,
                    *Task->MatchmakingLobbyId,
                    *Task->Request.UserId);
                Txn->SetMetadata.Add(MatchmakingMemberMetadataConnecting, true);
                if (!Pins->OnlineLobby->UpdateMemberSelf(
                        *Task->Request.UserId,
                        *Task->MatchmakingLobbyId,
                        *Txn,
                        FOnLobbyOperationComplete::CreateSP(
                            this,
                            &FMatchmakingAlgorithmStepWaitForSessionIdOrConnectionString::OnMarkedAsConnecting,
                            TWeakPtr<FMatchmakingEngineTask>(Task),
                            NextStep,
                            ConnectionUrlOrSessionId)))
                {
                    this->RestartableError(
                        *Task,
                        TEXT(
                            "Unable to tell the host that we're connecting on the client during retry for timed_out!"));
                }
                return;
            }
        }

        this->RestartableError(
            *Task,
            FString::Printf(
                TEXT("Unable to tell the host that we're connecting to the game server on the client: %s"),
                *Error.ToLogString()));
        return;
    }

    UE_LOG(
        LogMatchmaking,
        Verbose,
        TEXT("%sFinished informing host that we're now connecting to the game server"),
        *this->GetLogPrefix(*Task));

    if (NextStep.IsEqual(MMSTEP_CONNECT_TO_GAME_SERVER))
    {
        UE_LOG(
            LogMatchmaking,
            Verbose,
            TEXT("%sRequested to connect to game server: %s"),
            *this->GetLogPrefix(*Task),
            *ConnectionUrlOrSessionId);

        this->MoveToStep(*Task, MMSTEP_CONNECT_TO_GAME_SERVER, ConnectionUrlOrSessionId);
    }
    else
    {
        UE_LOG(
            LogMatchmaking,
            Verbose,
            TEXT("%sRequested to join session: %s"),
            *this->GetLogPrefix(*Task),
            *ConnectionUrlOrSessionId);

        this->MoveToStep(*Task, MMSTEP_JOIN_SERVER_SESSION, ConnectionUrlOrSessionId);
    }
}

#endif

REDPOINT_EOS_CODE_GUARD_END()