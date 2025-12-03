// Copyright June Rhodes. All Rights Reserved.

#include "MatchmakingEventHandlerHostHeartbeat.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "../Algorithm/MatchmakingAlgorithmStep.h"
#include "RedpointMatchmaking/MatchmakingConstants.h"
#include "RedpointMatchmaking/MatchmakingLog.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMHANDLER_HOST_HEARTBEAT FName("EventHandlerHostHeartbeat")

void FMatchmakingEventHandlerHostHeartbeat::OnLobbyUpdate(
    const FUniqueNetId &UserId,
    const FOnlineLobbyId &LobbyId,
    TSharedRef<FMatchmakingEngineImpl> Engine,
    TWeakPtr<FMatchmakingEngineTask> TaskWk)
{
    auto Task = TaskWk.Pin();
    check(Task.IsValid());

    EMIT_TASK_COUNTER_TRACE(Task);

    UE_LOG(
        LogMatchmaking,
        Verbose,
        TEXT("%sGot OnLobbyUpdate event in FMatchmakingEventHandlerHostHeartbeat"),
        *this->GetLogPrefix(*Task, MMHANDLER_HOST_HEARTBEAT));

    if (!Task->MatchmakingLobbyId.IsValid())
    {
        UE_LOG(
            LogMatchmaking,
            Verbose,
            TEXT("%sSkipping processing because the matchmaking lobby ID isn't valid"),
            *this->GetLogPrefix(*Task, MMHANDLER_HOST_HEARTBEAT));
        return;
    }
    if (*Task->MatchmakingLobbyId != LobbyId)
    {
        UE_LOG(
            LogMatchmaking,
            Verbose,
            TEXT("%sSkipping processing because the matchmaking lobby ID '%s' doesn't match the lobby the event is for "
                 "'%s'"),
            *this->GetLogPrefix(*Task, MMHANDLER_HOST_HEARTBEAT),
            *Task->MatchmakingLobbyId->ToString(),
            *LobbyId.ToString());
        return;
    }
    if (*Task->Request.UserId != UserId)
    {
        UE_LOG(
            LogMatchmaking,
            Verbose,
            TEXT("%sSkipping processing because the request user ID '%s' doesn't match the user the event is for "
                 "'%s'"),
            *this->GetLogPrefix(*Task, MMHANDLER_HOST_HEARTBEAT),
            *Task->Request.UserId->ToString(),
            *UserId.ToString());
        return;
    }

    auto Step = Engine->GetRegisteredStep<IMatchmakingAlgorithmStep<IMatchmakingAlgorithmStepState>>(Task->CurrentStep);
    if (!Step->CanHandleInterrupt(Task.ToSharedRef(), MMINTERRUPT_CLIENT_GOT_HOST_HEARTBEAT))
    {
        UE_LOG(
            LogMatchmaking,
            Verbose,
            TEXT("%sThe current step can't handle MMINTERRUPT_CLIENT_GOT_HOST_HEARTBEAT interrupt, so we're not "
                 "processing the event"),
            *this->GetLogPrefix(*Task, MMHANDLER_HOST_HEARTBEAT));
        return;
    }

    if (auto Pins = Task->PinAndContinue())
    {
        FVariantData LobbyHeartbeatData;
        if (Pins->OnlineLobby->GetLobbyMetadataValue(
                UserId,
                *Task->MatchmakingLobbyId,
                MatchmakingMetadataLobbyHeartbeatTimestamp,
                LobbyHeartbeatData))
        {
            if (LobbyHeartbeatData.GetType() == EOnlineKeyValuePairDataType::Int64)
            {
                int64 LobbyHeartbeatValue;
                LobbyHeartbeatData.GetValue(LobbyHeartbeatValue);

                if (LobbyHeartbeatValue != Task->LastHostHeartbeatValue)
                {
                    Task->LastHostHeartbeatTime = FDateTime::UtcNow();
                    Task->LastHostHeartbeatValue = LobbyHeartbeatValue;
                    UE_LOG(
                        LogMatchmaking,
                        Verbose,
                        TEXT("%sFiring MMINTERRUPT_CLIENT_GOT_HOST_HEARTBEAT for matchmaking lobby %s"),
                        *this->GetLogPrefix(*Task, MMHANDLER_HOST_HEARTBEAT),
                        *Task->MatchmakingLobbyId->ToString());
                    verify(Step->HandleInterrupt(Task.ToSharedRef(), MMINTERRUPT_CLIENT_GOT_HOST_HEARTBEAT));
                }
                else
                {
                    UE_LOG(
                        LogMatchmaking,
                        Verbose,
                        TEXT("%sHost heartbeat didn't change"),
                        *this->GetLogPrefix(*Task, MMHANDLER_HOST_HEARTBEAT));
                }
            }
            else
            {
                UE_LOG(
                    LogMatchmaking,
                    Verbose,
                    TEXT("%sThe MatchmakingMetadataLobbyHeartbeatTimestamp key wasn't a Int64"),
                    *this->GetLogPrefix(*Task, MMHANDLER_HOST_HEARTBEAT));
            }
        }
        else
        {
            UE_LOG(
                LogMatchmaking,
                Verbose,
                TEXT("%sCouldn't retrieve MatchmakingMetadataLobbyHeartbeatTimestamp key"),
                *this->GetLogPrefix(*Task, MMHANDLER_HOST_HEARTBEAT));
        }
    }
    else
    {
        UE_LOG(
            LogMatchmaking,
            Verbose,
            TEXT("%sThe interfaces couldn't be pinned"),
            *this->GetLogPrefix(*Task, MMHANDLER_HOST_HEARTBEAT));
    }
}

void FMatchmakingEventHandlerHostHeartbeat::RegisterEvents(FMatchmakingEngineImpl &Engine, FMatchmakingEngineTask &Task)
{
    if (auto Pins = Task.PinAndContinue())
    {
        Task.EventHandlers.Add(
            MMHANDLER_HOST_HEARTBEAT,
            Pins->OnlineLobby->AddOnLobbyUpdateDelegate_Handle(FOnLobbyUpdateDelegate::CreateSP(
                this,
                &FMatchmakingEventHandlerHostHeartbeat::OnLobbyUpdate,
                Engine.AsShared(),
                TWeakPtr<FMatchmakingEngineTask>(Task.AsShared()))));
        EMIT_TASK_COUNTER_TRACE(Task.AsShared());
    }
}

void FMatchmakingEventHandlerHostHeartbeat::UnregisterEvents(
    FMatchmakingEngineImpl &Engine,
    FMatchmakingEngineTask &Task)
{
    if (auto Pins = Task.PinAndContinue())
    {
        if (Task.EventHandlers.Contains(MMHANDLER_HOST_HEARTBEAT))
        {
            Pins->OnlineLobby->ClearOnLobbyUpdateDelegate_Handle(Task.EventHandlers[MMHANDLER_HOST_HEARTBEAT]);
            Task.EventHandlers.Remove(MMHANDLER_HOST_HEARTBEAT);
            EMIT_TASK_COUNTER_TRACE(Task.AsShared());
        }
    }
}

#endif

REDPOINT_EOS_CODE_GUARD_END()