// Copyright June Rhodes. All Rights Reserved.

#include "MatchmakingEventHandlerClientProgressEmit.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "../Algorithm/MatchmakingAlgorithmStep.h"
#include "RedpointMatchmaking/MatchmakingConstants.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMHANDLER_CLIENT_PROGRESS_EMIT_CONNECT FName("EventHandlerClientProgressEmitConnect")
#define MMHANDLER_CLIENT_PROGRESS_EMIT_DISCONNECT FName("EventHandlerClientProgressEmitDisconnect")

void FMatchmakingEventHandlerClientProgressEmit::OnMemberConnect(
    const FUniqueNetId &UserId,
    const FOnlineLobbyId &LobbyId,
    const FUniqueNetId &MemberId,
    TSharedRef<FMatchmakingEngineImpl> Engine,
    TWeakPtr<FMatchmakingEngineTask> TaskWk)
{
    auto Task = TaskWk.Pin();
    check(Task.IsValid());

    EMIT_TASK_COUNTER_TRACE(Task);

    if (!Task->TeamManager.IsValid() && Task->MatchmakingLobbyId.IsValid() && *Task->MatchmakingLobbyId == LobbyId &&
        *Task->Request.UserId == UserId)
    {
        if (Task->Request.OnProgress.IsBound())
        {
            auto Step =
                Engine->GetRegisteredStep<IMatchmakingAlgorithmStep<IMatchmakingAlgorithmStepState>>(Task->CurrentStep);
            FMatchmakingEngineProgressInfo ProgressInfo = {};
            ProgressInfo.Handle = Task->Handle;
            ProgressInfo.UserId = Task->Request.UserId;
            ProgressInfo.PartyId = Task->Request.PartyId;
            ProgressInfo.StepName = Step->GetName();
            ProgressInfo.CurrentStatus = Step->GetStatus();
            ProgressInfo.CurrentDetail = Step->GetDetail();
            ProgressInfo.CurrentProgress = FMatchmakingProgressCalculator::ComputeProgress(*Task);
            ProgressInfo.EstimatedTimeOfCompletion = Task->EstimatedTimeOfCompletion;
            Task->Request.OnProgress.ExecuteIfBound(Task->TaskId, Task->Request, ProgressInfo);
            EMIT_TASK_COUNTER_TRACE(Task->AsShared());
        }
    }
}

void FMatchmakingEventHandlerClientProgressEmit::OnMemberDisconnect(
    const FUniqueNetId &UserId,
    const FOnlineLobbyId &LobbyId,
    const FUniqueNetId &MemberId,
    bool bWasKicked,
    TSharedRef<FMatchmakingEngineImpl> Engine,
    TWeakPtr<FMatchmakingEngineTask> TaskWk)
{
    auto Task = TaskWk.Pin();
    check(Task.IsValid());

    EMIT_TASK_COUNTER_TRACE(Task);

    if (!Task->TeamManager.IsValid() && Task->MatchmakingLobbyId.IsValid() && *Task->MatchmakingLobbyId == LobbyId &&
        *Task->Request.UserId == UserId)
    {
        if (Task->Request.OnProgress.IsBound())
        {
            auto Step =
                Engine->GetRegisteredStep<IMatchmakingAlgorithmStep<IMatchmakingAlgorithmStepState>>(Task->CurrentStep);
            FMatchmakingEngineProgressInfo ProgressInfo = {};
            ProgressInfo.Handle = Task->Handle;
            ProgressInfo.UserId = Task->Request.UserId;
            ProgressInfo.PartyId = Task->Request.PartyId;
            ProgressInfo.StepName = Step->GetName();
            ProgressInfo.CurrentStatus = Step->GetStatus();
            ProgressInfo.CurrentDetail = Step->GetDetail();
            ProgressInfo.CurrentProgress = FMatchmakingProgressCalculator::ComputeProgress(*Task);
            ProgressInfo.EstimatedTimeOfCompletion = Task->EstimatedTimeOfCompletion;
            Task->Request.OnProgress.ExecuteIfBound(Task->TaskId, Task->Request, ProgressInfo);
            EMIT_TASK_COUNTER_TRACE(Task->AsShared());
        }
    }
}

void FMatchmakingEventHandlerClientProgressEmit::RegisterEvents(
    FMatchmakingEngineImpl &Engine,
    FMatchmakingEngineTask &Task)
{
    if (auto Pins = Task.PinAndContinue())
    {
        Task.EventHandlers.Add(
            MMHANDLER_CLIENT_PROGRESS_EMIT_CONNECT,
            Pins->OnlineLobby->AddOnMemberConnectDelegate_Handle(FOnMemberConnectDelegate::CreateSP(
                this,
                &FMatchmakingEventHandlerClientProgressEmit::OnMemberConnect,
                Engine.AsShared(),
                TWeakPtr<FMatchmakingEngineTask>(Task.AsShared()))));
        Task.EventHandlers.Add(
            MMHANDLER_CLIENT_PROGRESS_EMIT_DISCONNECT,
            Pins->OnlineLobby->AddOnMemberDisconnectDelegate_Handle(FOnMemberDisconnectDelegate::CreateSP(
                this,
                &FMatchmakingEventHandlerClientProgressEmit::OnMemberDisconnect,
                Engine.AsShared(),
                TWeakPtr<FMatchmakingEngineTask>(Task.AsShared()))));
        EMIT_TASK_COUNTER_TRACE(Task.AsShared());
    }
}

void FMatchmakingEventHandlerClientProgressEmit::UnregisterEvents(
    FMatchmakingEngineImpl &Engine,
    FMatchmakingEngineTask &Task)
{
    if (auto Pins = Task.PinAndContinue())
    {
        if (Task.EventHandlers.Contains(MMHANDLER_CLIENT_PROGRESS_EMIT_CONNECT))
        {
            Pins->OnlineLobby->ClearOnMemberConnectDelegate_Handle(
                Task.EventHandlers[MMHANDLER_CLIENT_PROGRESS_EMIT_CONNECT]);
            Task.EventHandlers.Remove(MMHANDLER_CLIENT_PROGRESS_EMIT_CONNECT);
            EMIT_TASK_COUNTER_TRACE(Task.AsShared());
        }
        if (Task.EventHandlers.Contains(MMHANDLER_CLIENT_PROGRESS_EMIT_DISCONNECT))
        {
            Pins->OnlineLobby->ClearOnMemberDisconnectDelegate_Handle(
                Task.EventHandlers[MMHANDLER_CLIENT_PROGRESS_EMIT_DISCONNECT]);
            Task.EventHandlers.Remove(MMHANDLER_CLIENT_PROGRESS_EMIT_DISCONNECT);
            EMIT_TASK_COUNTER_TRACE(Task.AsShared());
        }
    }
}

#endif

REDPOINT_EOS_CODE_GUARD_END()