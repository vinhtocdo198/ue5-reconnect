// Copyright June Rhodes. All Rights Reserved.

#include "MatchmakingEventHandlerKicked.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "../Algorithm/MatchmakingAlgorithmStep.h"
#include "../Algorithm/MatchmakingAlgorithmStepAbandonStart.h"
#include "MatchmakingEventHandlerAbandonment.h"
#include "RedpointMatchmaking/MatchmakingConstants.h"
#include "RedpointMatchmaking/MatchmakingLog.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMHANDLER_KICKED FName("EventHandlerKicked")

void FMatchmakingEventHandlerKicked::OnLobbyMemberDisconnect(
    const FUniqueNetId &UserId,
    const FOnlineLobbyId &LobbyId,
    const FUniqueNetId &MemberId,
    bool bWasKicked,
    TSharedRef<FMatchmakingEngineImpl> Engine,
    TWeakPtr<FMatchmakingEngineTask> TaskWk)
{
    auto Task = TaskWk.Pin();
    check(Task.IsValid());

    if (UserId != *Task->Request.UserId || MemberId != *Task->Request.UserId || !bWasKicked)
    {
        return;
    }

    // If we have a stored response, we're beyond the point where we care about being kicked.
    if (Task->StoredResponse.IsValid())
    {
        UE_LOG(
            LogMatchmaking,
            Verbose,
            TEXT("%sIgnoring kick from lobby, because we already have a finalized response."),
            *this->GetLogPrefix(*Task, MMHANDLER_KICKED),
            *LobbyId.ToString());
        return;
    }

    // We got kicked from a lobby (doesn't matter which one). Abandon and restart.
    UE_LOG(
        LogMatchmaking,
        Verbose,
        TEXT("%sGot kicked from lobby %s, restarting matchmaking..."),
        *this->GetLogPrefix(*Task, MMHANDLER_KICKED),
        *LobbyId.ToString());

    auto Step = Engine->GetRegisteredStep<IMatchmakingAlgorithmStep<IMatchmakingAlgorithmStepState>>(Task->CurrentStep);
    if (Step->CanHandleInterrupt(Task.ToSharedRef(), MMINTERRUPT_HAS_SPECIAL_HANDLING_OF_ABANDONMENT))
    {
        if (!Step->HandleInterrupt(Task.ToSharedRef(), MMINTERRUPT_ABANDON_RESTART))
        {
            Step->MoveToStep(*Task, MMSTEP_ABANDON_START, EMatchmakingAbandonmentType::Restart);
        }
    }
    else
    {
        Step->MoveToStep(*Task, MMSTEP_ABANDON_START, EMatchmakingAbandonmentType::Restart);
    }
}

void FMatchmakingEventHandlerKicked::RegisterEvents(FMatchmakingEngineImpl &Engine, FMatchmakingEngineTask &Task)
{
    if (auto Pins = Task.PinAndContinue())
    {
        Task.EventHandlers.Add(
            MMHANDLER_KICKED,
            Pins->OnlineLobby->AddOnMemberDisconnectDelegate_Handle(FOnMemberDisconnectDelegate::CreateSP(
                this,
                &FMatchmakingEventHandlerKicked::OnLobbyMemberDisconnect,
                Engine.AsShared(),
                TWeakPtr<FMatchmakingEngineTask>(Task.AsShared()))));
        EMIT_TASK_COUNTER_TRACE(Task.AsShared());
    }
}

void FMatchmakingEventHandlerKicked::UnregisterEvents(FMatchmakingEngineImpl &Engine, FMatchmakingEngineTask &Task)
{
    if (auto Pins = Task.PinAndContinue())
    {
        if (Task.EventHandlers.Contains(MMHANDLER_KICKED))
        {
            Pins->OnlineLobby->ClearOnMemberDisconnectDelegate_Handle(Task.EventHandlers[MMHANDLER_KICKED]);
            Task.EventHandlers.Remove(MMHANDLER_KICKED);
        }

        EMIT_TASK_COUNTER_TRACE(Task.AsShared());
    }
}

#endif

REDPOINT_EOS_CODE_GUARD_END()