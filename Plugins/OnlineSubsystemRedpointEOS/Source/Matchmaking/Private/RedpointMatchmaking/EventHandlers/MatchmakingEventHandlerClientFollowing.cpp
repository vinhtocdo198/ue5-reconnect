// Copyright June Rhodes. All Rights Reserved.

#include "MatchmakingEventHandlerClientFollowing.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "../Algorithm/MatchmakingAlgorithmStep.h"
#include "../Algorithm/MatchmakingAlgorithmStepIncomingFollowRequestReject.h"
#include "../Algorithm/MatchmakingAlgorithmStepIncomingFollowRequestVerify.h"
#include "../MatchmakingFollowRequest.h"
#include "../MatchmakingTeamManager.h"
#include "RedpointMatchmaking/MatchmakingConstants.h"
#include "RedpointMatchmaking/MatchmakingLog.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMHANDLER_CLIENT_FOLLOWING FName("EventHandlerClientFollowing")

void FMatchmakingEventHandlerClientFollowing::OnLobbyUpdate(
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
        VeryVerbose,
        TEXT("%sGot OnLobbyUpdate event in FMatchmakingEventHandlerClientFollowing"),
        *this->GetLogPrefix(*Task, MMHANDLER_CLIENT_FOLLOWING));

    if (!Task->MatchmakingLobbyId.IsValid())
    {
        UE_LOG(
            LogMatchmaking,
            VeryVerbose,
            TEXT("%sSkipping processing because the matchmaking lobby ID isn't valid"),
            *this->GetLogPrefix(*Task, MMHANDLER_CLIENT_FOLLOWING));
        return;
    }
    if (*Task->MatchmakingLobbyId != LobbyId)
    {
        UE_LOG(
            LogMatchmaking,
            VeryVerbose,
            TEXT("%sSkipping processing because the matchmaking lobby ID '%s' doesn't match the lobby the event is for "
                 "'%s'"),
            *this->GetLogPrefix(*Task, MMHANDLER_CLIENT_FOLLOWING),
            *Task->MatchmakingLobbyId->ToString(),
            *LobbyId.ToString());
        return;
    }
    if (*Task->Request.UserId != UserId)
    {
        UE_LOG(
            LogMatchmaking,
            VeryVerbose,
            TEXT("%sSkipping processing because the request user ID '%s' doesn't match the user the event is for "
                 "'%s'"),
            *this->GetLogPrefix(*Task, MMHANDLER_CLIENT_FOLLOWING),
            *Task->Request.UserId->ToString(),
            *UserId.ToString());
        return;
    }

    auto Step = Engine->GetRegisteredStep<IMatchmakingAlgorithmStep<IMatchmakingAlgorithmStepState>>(Task->CurrentStep);
    if (!Step->CanHandleInterrupt(Task.ToSharedRef(), MMINTERRUPT_CLIENT_FOLLOWED_LOBBY_COMPLETED_MATCH) ||
        !Step->CanHandleInterrupt(Task.ToSharedRef(), MMINTERRUPT_CLIENT_FOLLOWED_LOBBY_NOW_FOLLOWING_ANOTHER_LOBBY))
    {
        UE_LOG(
            LogMatchmaking,
            VeryVerbose,
            TEXT("%sThe current step can't handle MMINTERRUPT_CLIENT_FOLLOWED_LOBBY_COMPLETED_MATCH or "
                 "MMINTERRUPT_CLIENT_FOLLOWED_LOBBY_NOW_FOLLOWING_ANOTHER_LOBBY interrupts, so we're not processing "
                 "the event"),
            *this->GetLogPrefix(*Task, MMHANDLER_CLIENT_FOLLOWING));
        return;
    }

    if (auto Pins = Task->PinAndContinue())
    {
        FVariantData LobbyStatusData;
        if (Pins->OnlineLobby->GetLobbyMetadataValue(
                UserId,
                *Task->MatchmakingLobbyId,
                MatchmakingMetadataLobbyStatus,
                LobbyStatusData))
        {
            if (LobbyStatusData.GetType() == EOnlineKeyValuePairDataType::String)
            {
                FString LobbyStatus;
                LobbyStatusData.GetValue(LobbyStatus);
                UE_LOG(LogMatchmaking, VeryVerbose, TEXT("The lobby is currently in status '%s'"), *LobbyStatus);
                if (LobbyStatus == MatchmakingStatusFilling || LobbyStatus == MatchmakingStatusSearching ||
                    LobbyStatus == MatchmakingStatusWaiting)
                {
                    // These are "nothing" states from the perspective of someone following a session.
                    UE_LOG(
                        LogMatchmaking,
                        VeryVerbose,
                        TEXT("%sNot an interesting status that we care about"),
                        *this->GetLogPrefix(*Task, MMHANDLER_CLIENT_FOLLOWING));
                }
                else if (LobbyStatus == MatchmakingStatusFollowing)
                {
                    // The lobby we are following is now following another session. Disconnect from the current followed
                    // lobby and follow the new one.
                    UE_LOG(
                        LogMatchmaking,
                        VeryVerbose,
                        TEXT("%sFiring MMINTERRUPT_CLIENT_FOLLOWED_LOBBY_NOW_FOLLOWING_ANOTHER_LOBBY"),
                        *this->GetLogPrefix(*Task, MMHANDLER_CLIENT_FOLLOWING));
                    verify(Step->HandleInterrupt(
                        Task.ToSharedRef(),
                        MMINTERRUPT_CLIENT_FOLLOWED_LOBBY_NOW_FOLLOWING_ANOTHER_LOBBY));
                }
                else if (LobbyStatus == MatchmakingStatusFinalizing)
                {
                    // The lobby is finalizing, and the client needs to finalize it's result.
                    UE_LOG(
                        LogMatchmaking,
                        VeryVerbose,
                        TEXT("%sFiring MMINTERRUPT_CLIENT_FOLLOWED_LOBBY_COMPLETED_MATCH"),
                        *this->GetLogPrefix(*Task, MMHANDLER_CLIENT_FOLLOWING));
                    verify(
                        Step->HandleInterrupt(Task.ToSharedRef(), MMINTERRUPT_CLIENT_FOLLOWED_LOBBY_COMPLETED_MATCH));
                }
            }
            else
            {
                UE_LOG(
                    LogMatchmaking,
                    VeryVerbose,
                    TEXT("%sThe MatchmakingMetadataLobbyStatus key wasn't a string"),
                    *this->GetLogPrefix(*Task, MMHANDLER_CLIENT_FOLLOWING));
            }
        }
        else
        {
            UE_LOG(
                LogMatchmaking,
                VeryVerbose,
                TEXT("%sCouldn't retrieve MatchmakingMetadataLobbyStatus key"),
                *this->GetLogPrefix(*Task, MMHANDLER_CLIENT_FOLLOWING));
        }
    }
    else
    {
        UE_LOG(
            LogMatchmaking,
            VeryVerbose,
            TEXT("%sThe interfaces couldn't be pinned"),
            *this->GetLogPrefix(*Task, MMHANDLER_CLIENT_FOLLOWING));
    }
}

void FMatchmakingEventHandlerClientFollowing::RegisterEvents(
    FMatchmakingEngineImpl &Engine,
    FMatchmakingEngineTask &Task)
{
    if (auto Pins = Task.PinAndContinue())
    {
        Task.EventHandlers.Add(
            MMHANDLER_CLIENT_FOLLOWING,
            Pins->OnlineLobby->AddOnLobbyUpdateDelegate_Handle(FOnLobbyUpdateDelegate::CreateSP(
                this,
                &FMatchmakingEventHandlerClientFollowing::OnLobbyUpdate,
                Engine.AsShared(),
                TWeakPtr<FMatchmakingEngineTask>(Task.AsShared()))));
        EMIT_TASK_COUNTER_TRACE(Task.AsShared());
    }
}

void FMatchmakingEventHandlerClientFollowing::UnregisterEvents(
    FMatchmakingEngineImpl &Engine,
    FMatchmakingEngineTask &Task)
{
    if (auto Pins = Task.PinAndContinue())
    {
        if (Task.EventHandlers.Contains(MMHANDLER_CLIENT_FOLLOWING))
        {
            Pins->OnlineLobby->ClearOnLobbyUpdateDelegate_Handle(Task.EventHandlers[MMHANDLER_CLIENT_FOLLOWING]);
            Task.EventHandlers.Remove(MMHANDLER_CLIENT_FOLLOWING);
            EMIT_TASK_COUNTER_TRACE(Task.AsShared());
        }
    }
}

#endif

REDPOINT_EOS_CODE_GUARD_END()