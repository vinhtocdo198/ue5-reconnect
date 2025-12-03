// Copyright June Rhodes. All Rights Reserved.

#include "MatchmakingEventHandlerConnectionInfoProvided.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "../Algorithm/MatchmakingAlgorithmStep.h"
#include "../MatchmakingFollowRequest.h"
#include "../MatchmakingFollowResponse.h"
#include "../MatchmakingStateHash.h"
#include "RedpointMatchmaking/MatchmakingConstants.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMHANDLER_CONNECTION_INFO_PROVIDED FName("EventHandlerConnectionInfoProvided")

void FMatchmakingEventHandlerConnectionInfoProvided::OnLobbyUpdate(
    const FUniqueNetId &UserId,
    const FOnlineLobbyId &LobbyId,
    TSharedRef<FMatchmakingEngineImpl> Engine,
    TWeakPtr<FMatchmakingEngineTask> TaskWk)
{
    auto Task = TaskWk.Pin();
    check(Task.IsValid());

    EMIT_TASK_COUNTER_TRACE(Task);

    if (Task->MatchmakingLobbyId.IsValid() && *Task->MatchmakingLobbyId == LobbyId && *Task->Request.UserId == UserId)
    {
        if (auto Pins = Task->PinAndContinue())
        {
            FVariantData Data;
            if (Pins->OnlineLobby
                    ->GetLobbyMetadataValue(UserId, LobbyId, MatchmakingMetadataConnectionSessionId, Data) &&
                Data.GetType() == EOnlineKeyValuePairDataType::String)
            {
                FString ConnectionSessionId;
                Data.GetValue(ConnectionSessionId);

                UE_LOG(
                    LogMatchmaking,
                    Verbose,
                    TEXT("%sReceived session ID to connect to: %s"),
                    *this->GetLogPrefix(*Task, MMHANDLER_CONNECTION_INFO_PROVIDED),
                    *ConnectionSessionId);

                auto Step = Engine->GetRegisteredStep<IMatchmakingAlgorithmStep<IMatchmakingAlgorithmStepState>>(
                    Task->CurrentStep);
                if (Step->CanHandleInterrupt(Task.ToSharedRef(), MMINTERRUPT_RECEIVED_CONNECTION_SESSION_ID))
                {
                    verifyf(
                        Step->HandleInterrupt(Task.ToSharedRef(), MMINTERRUPT_RECEIVED_CONNECTION_SESSION_ID),
                        TEXT("Step must handle connection session ID if it advertises it can"));
                }
            }
            else if (
                Pins->OnlineLobby->GetLobbyMetadataValue(UserId, LobbyId, MatchmakingMetadataConnectionUrl, Data) &&
                Data.GetType() == EOnlineKeyValuePairDataType::String)
            {
                FString ConnectionUrl;
                Data.GetValue(ConnectionUrl);

                UE_LOG(
                    LogMatchmaking,
                    Verbose,
                    TEXT("%sReceived URL to connect to: %s"),
                    *this->GetLogPrefix(*Task, MMHANDLER_CONNECTION_INFO_PROVIDED),
                    *ConnectionUrl);

                auto Step = Engine->GetRegisteredStep<IMatchmakingAlgorithmStep<IMatchmakingAlgorithmStepState>>(
                    Task->CurrentStep);
                if (Step->CanHandleInterrupt(Task.ToSharedRef(), MMINTERRUPT_RECEIVED_CONNECTION_URL))
                {
                    verifyf(
                        Step->HandleInterrupt(Task.ToSharedRef(), MMINTERRUPT_RECEIVED_CONNECTION_URL),
                        TEXT("Step must handle connection URL if it advertises it can"));
                }
            }
        }
    }

    EMIT_TASK_COUNTER_TRACE(Task);
}

void FMatchmakingEventHandlerConnectionInfoProvided::RegisterEvents(
    FMatchmakingEngineImpl &Engine,
    FMatchmakingEngineTask &Task)
{
    if (auto Pins = Task.PinAndContinue())
    {
        Task.EventHandlers.Add(
            MMHANDLER_CONNECTION_INFO_PROVIDED,
            Pins->OnlineLobby->AddOnLobbyUpdateDelegate_Handle(FOnLobbyUpdateDelegate::CreateSP(
                this,
                &FMatchmakingEventHandlerConnectionInfoProvided::OnLobbyUpdate,
                Engine.AsShared(),
                TWeakPtr<FMatchmakingEngineTask>(Task.AsShared()))));
        EMIT_TASK_COUNTER_TRACE(Task.AsShared());
    }
}

void FMatchmakingEventHandlerConnectionInfoProvided::UnregisterEvents(
    FMatchmakingEngineImpl &Engine,
    FMatchmakingEngineTask &Task)
{
    if (auto Pins = Task.PinAndContinue())
    {
        if (Task.EventHandlers.Contains(MMHANDLER_CONNECTION_INFO_PROVIDED))
        {
            Pins->OnlineLobby->ClearOnLobbyUpdateDelegate_Handle(
                Task.EventHandlers[MMHANDLER_CONNECTION_INFO_PROVIDED]);
            Task.EventHandlers.Remove(MMHANDLER_CONNECTION_INFO_PROVIDED);
            EMIT_TASK_COUNTER_TRACE(Task.AsShared());
        }
    }
}

#endif

REDPOINT_EOS_CODE_GUARD_END()