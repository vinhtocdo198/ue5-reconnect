// Copyright June Rhodes. All Rights Reserved.

#include "MatchmakingEngineImpl.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "Containers/Ticker.h"
#include "RedpointMatchmaking/MatchmakingConstants.h"
#include "RedpointMatchmaking/MatchmakingLog.h"

#include "Algorithm/MatchmakingAlgorithmStepAbandonFinish.h"
#include "Algorithm/MatchmakingAlgorithmStepAbandonNotifyMatch.h"
#include "Algorithm/MatchmakingAlgorithmStepAbandonNotifyParty.h"
#include "Algorithm/MatchmakingAlgorithmStepAbandonStart.h"
#include "Algorithm/MatchmakingAlgorithmStepAbandonWaitForPartyLeader.h"
#include "Algorithm/MatchmakingAlgorithmStepChainFollowOntoNewHost.h"
#include "Algorithm/MatchmakingAlgorithmStepCompleteMatch.h"
#include "Algorithm/MatchmakingAlgorithmStepCompleteMatchAsClient.h"
#include "Algorithm/MatchmakingAlgorithmStepCompleteMatchAsHost.h"
#include "Algorithm/MatchmakingAlgorithmStepCompleteMatchComputeResult.h"
#include "Algorithm/MatchmakingAlgorithmStepCompleteMatchFinalizeLobby.h"
#include "Algorithm/MatchmakingAlgorithmStepCompleteMatchFireEvent.h"
#include "Algorithm/MatchmakingAlgorithmStepConnectToGameServer.h"
#include "Algorithm/MatchmakingAlgorithmStepConnectionInformationSentForDedicatedServer.h"
#include "Algorithm/MatchmakingAlgorithmStepConnectionInformationSentForListenServer.h"
#include "Algorithm/MatchmakingAlgorithmStepCreateMatchmakingFollowRequestLobby.h"
#include "Algorithm/MatchmakingAlgorithmStepCreateMatchmakingLobby.h"
#include "Algorithm/MatchmakingAlgorithmStepCreateSessionForListenServer.h"
#include "Algorithm/MatchmakingAlgorithmStepDeleteMatchmakingFollowRequestLobby.h"
#include "Algorithm/MatchmakingAlgorithmStepDeleteMatchmakingLobby.h"
#include "Algorithm/MatchmakingAlgorithmStepError.h"
#include "Algorithm/MatchmakingAlgorithmStepFindDedicatedServerSession.h"
#include "Algorithm/MatchmakingAlgorithmStepFollowHost.h"
#include "Algorithm/MatchmakingAlgorithmStepIncomingFollowRequestAccept.h"
#include "Algorithm/MatchmakingAlgorithmStepIncomingFollowRequestReject.h"
#include "Algorithm/MatchmakingAlgorithmStepIncomingFollowRequestVerify.h"
#include "Algorithm/MatchmakingAlgorithmStepJoinServerSession.h"
#include "Algorithm/MatchmakingAlgorithmStepLockParty.h"
#include "Algorithm/MatchmakingAlgorithmStepNotifyExistingParty.h"
#include "Algorithm/MatchmakingAlgorithmStepReserveDedicatedServerForMatch.h"
#include "Algorithm/MatchmakingAlgorithmStepSearching.h"
#include "Algorithm/MatchmakingAlgorithmStepSearchingMoveToSearchingStatus.h"
#include "Algorithm/MatchmakingAlgorithmStepSearchingPerformSearch.h"
#include "Algorithm/MatchmakingAlgorithmStepSearchingSendFollowRequest.h"
#include "Algorithm/MatchmakingAlgorithmStepSearchingSendFollowRequestDoRequest.h"
#include "Algorithm/MatchmakingAlgorithmStepSearchingSendFollowRequestMoveToWaitingStatus.h"
#include "Algorithm/MatchmakingAlgorithmStepSendSessionIdOrConnectionString.h"
#include "Algorithm/MatchmakingAlgorithmStepSendSessionIdOrConnectionStringForListenServer.h"
#include "Algorithm/MatchmakingAlgorithmStepStartListenServer.h"
#include "Algorithm/MatchmakingAlgorithmStepUnlockParty.h"
#include "Algorithm/MatchmakingAlgorithmStepValidateRequest.h"
#include "Algorithm/MatchmakingAlgorithmStepWaitForPartyLeader.h"
#include "Algorithm/MatchmakingAlgorithmStepWaitForSessionIdOrConnectionString.h"
#include "Algorithm/MatchmakingAlgorithmStepWaitingForPlayersToJoin.h"

#include "EventHandlers/MatchmakingEventHandlerAbandonment.h"
#include "EventHandlers/MatchmakingEventHandlerClientFollowing.h"
#include "EventHandlers/MatchmakingEventHandlerClientProgressEmit.h"
#include "EventHandlers/MatchmakingEventHandlerConnectionInfoProvided.h"
#include "EventHandlers/MatchmakingEventHandlerFinalization.h"
#include "EventHandlers/MatchmakingEventHandlerHostHeartbeat.h"
#include "EventHandlers/MatchmakingEventHandlerKicked.h"
#include "EventHandlers/MatchmakingEventHandlerPartyChange.h"
#include "EventHandlers/MatchmakingEventHandlerReceiveFollowRequest.h"
#include "EventHandlers/MatchmakingEventHandlerTeamManagement.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMSTEP_INITIAL MMSTEP_VALIDATE_REQUEST

FMatchmakingEngineImpl::FMatchmakingEngineImpl(bool bInIsInstantScheduling)
    : NextHandle(1000)
    , RegisteredSteps()
    , QueuedTasks()
    , RegisteredEventHandlers()
    , bIsInstantScheduling(bInIsInstantScheduling)
    , ElapsedTime(0.0f)
    , ScheduledCallbacks()
{
}

void FMatchmakingEngineImpl::RegisterSteps()
{
    if (this->RegisteredSteps.Num() > 0)
    {
        return;
    }

    this->RegisterStep<FMatchmakingAlgorithmStepError>();
    this->RegisterStep<FMatchmakingAlgorithmStepValidateRequest>();
    this->RegisterStep<FMatchmakingAlgorithmStepLockParty>();
    this->RegisterStep<FMatchmakingAlgorithmStepCreateMatchmakingFollowRequestLobby>();
    this->RegisterStep<FMatchmakingAlgorithmStepCreateMatchmakingLobby>();
    this->RegisterStep<FMatchmakingAlgorithmStepNotifyExistingParty>();
    this->RegisterStep<FMatchmakingAlgorithmStepWaitingForPlayersToJoin>();
    this->RegisterStep<FMatchmakingAlgorithmStepSearching>();
    this->RegisterStep<FMatchmakingAlgorithmStepSearchingMoveToSearchingStatus>();
    this->RegisterStep<FMatchmakingAlgorithmStepSearchingPerformSearch>();
    this->RegisterStep<FMatchmakingAlgorithmStepSearchingSendFollowRequest>();
    this->RegisterStep<FMatchmakingAlgorithmStepSearchingSendFollowRequestMoveToWaitingStatus>();
    this->RegisterStep<FMatchmakingAlgorithmStepSearchingSendFollowRequestDoRequest>();
    this->RegisterStep<FMatchmakingAlgorithmStepIncomingFollowRequestVerify>();
    this->RegisterStep<FMatchmakingAlgorithmStepIncomingFollowRequestReject>();
    this->RegisterStep<FMatchmakingAlgorithmStepIncomingFollowRequestAccept>();
    this->RegisterStep<FMatchmakingAlgorithmStepCompleteMatch>();
    this->RegisterStep<FMatchmakingAlgorithmStepCompleteMatchFireEvent>();
    this->RegisterStep<FMatchmakingAlgorithmStepDeleteMatchmakingLobby>();
    this->RegisterStep<FMatchmakingAlgorithmStepDeleteMatchmakingFollowRequestLobby>();
    this->RegisterStep<FMatchmakingAlgorithmStepCompleteMatchFinalizeLobby>();
    this->RegisterStep<FMatchmakingAlgorithmStepCompleteMatchComputeResult>();
    this->RegisterStep<FMatchmakingAlgorithmStepFollowHost>();
    this->RegisterStep<FMatchmakingAlgorithmStepCompleteMatchAsClient>();
    this->RegisterStep<FMatchmakingAlgorithmStepCompleteUnlockParty>();
    this->RegisterStep<FMatchmakingAlgorithmStepCompleteMatchAsHost>();
    this->RegisterStep<FMatchmakingAlgorithmStepChainFollowOntoNewHost>();
    this->RegisterStep<FMatchmakingAlgorithmStepAbandonStart>();
    this->RegisterStep<FMatchmakingAlgorithmStepAbandonNotifyMatch>();
    this->RegisterStep<FMatchmakingAlgorithmStepAbandonUnlockParty>();
    this->RegisterStep<FMatchmakingAlgorithmStepAbandonNotifyParty>();
    this->RegisterStep<FMatchmakingAlgorithmStepAbandonFinish>();
    this->RegisterStep<FMatchmakingAlgorithmStepAbandonWaitForPartyLeader>();
    this->RegisterStep<FMatchmakingAlgorithmStepStartListenServer>();
    this->RegisterStep<FMatchmakingAlgorithmStepWaitForSessionIdOrConnectionString>();
    this->RegisterStep<FMatchmakingAlgorithmStepCreateSessionForListenServer>();
    this->RegisterStep<FMatchmakingAlgorithmStepFindDedicatedServerSession>();
    this->RegisterStep<FMatchmakingAlgorithmStepReserveDedicatedServerForMatch>();
    this->RegisterStep<FMatchmakingAlgorithmStepSendSessionIdOrConnectionString>();
    this->RegisterStep<FMatchmakingAlgorithmStepSendSessionIdOrConnectionStringForListenServer>();
    this->RegisterStep<FMatchmakingAlgorithmStepJoinServerSession>();
    this->RegisterStep<FMatchmakingAlgorithmStepConnectionInformationSentForDedicatedServer>();
    this->RegisterStep<FMatchmakingAlgorithmStepConnectionInformationSentForListenServer>();
    this->RegisterStep<FMatchmakingAlgorithmStepConnectToGameServer>();
    this->RegisterStep<FMatchmakingAlgorithmStepWaitForPartyLeader>();

    this->RegisteredEventHandlers.Add(MakeShared<FMatchmakingEventHandlerReceiveFollowRequest>());
    this->RegisteredEventHandlers.Add(MakeShared<FMatchmakingEventHandlerTeamManagement>());
    this->RegisteredEventHandlers.Add(MakeShared<FMatchmakingEventHandlerClientFollowing>());
    this->RegisteredEventHandlers.Add(MakeShared<FMatchmakingEventHandlerClientProgressEmit>());
    this->RegisteredEventHandlers.Add(MakeShared<FMatchmakingEventHandlerFinalization>());
    this->RegisteredEventHandlers.Add(MakeShared<FMatchmakingEventHandlerHostHeartbeat>());
    this->RegisteredEventHandlers.Add(MakeShared<FMatchmakingEventHandlerAbandonment>());
    this->RegisteredEventHandlers.Add(MakeShared<FMatchmakingEventHandlerPartyChange>());
    this->RegisteredEventHandlers.Add(MakeShared<FMatchmakingEventHandlerKicked>());
    this->RegisteredEventHandlers.Add(MakeShared<FMatchmakingEventHandlerConnectionInfoProvided>());
}

bool FMatchmakingEngineImpl::IsInstantScheduling()
{
    return this->bIsInstantScheduling;
}

FDelegateHandle FMatchmakingEngineImpl::OnSchedule(float Delay, FMatchmakingEngineTaskScheduleCallback Callback)
{
    FDelegateHandle Handle(FDelegateHandle::GenerateNewHandle);
    FScheduledCallback Cb;
    Cb.TargetElapsedTime = this->ElapsedTime + (this->bIsInstantScheduling ? 0.0f : Delay);
    Cb.DelegateHandle = Handle;
    Cb.Callback = MoveTemp(Callback);
    this->ScheduledCallbacks.Add(Cb);
    return Handle;
}

void FMatchmakingEngineImpl::OnUnschedule(FDelegateHandle Handle)
{
    for (int i = this->ScheduledCallbacks.Num() - 1; i >= 0; i--)
    {
        if (this->ScheduledCallbacks[i].DelegateHandle == Handle)
        {
            this->ScheduledCallbacks.RemoveAt(i);
        }
    }
}

bool FMatchmakingEngineImpl::OnHostSendHeartbeat(float DeltaSeconds, TWeakPtr<FMatchmakingEngineTask> TaskWk)
{
    auto Task = TaskWk.Pin();
    check(Task.IsValid());

    if (Task->MatchmakingLobbyId.IsValid() && Task->Request.UserId.IsValid())
    {
        if (Task->MatchmakingFollowRequestLobbyId.IsValid() /* Are we a host? */ &&
            Task->CurrentStep != MMSTEP_INCOMING_FOLLOW_REQUEST_ACCEPT /* Are we not accepting a request? */)
        {
            if (auto Pins = Task->PinAndContinue())
            {
                UE_LOG(
                    LogMatchmaking,
                    Verbose,
                    TEXT("Task %s: Heartbeating matchmaking lobby %s"),
                    *Task->TaskId,
                    *Task->MatchmakingLobbyId->ToString());
                auto Transaction =
                    Pins->OnlineLobby->MakeUpdateLobbyTransaction(*Task->Request.UserId, *Task->MatchmakingLobbyId);
                Transaction->SetMetadata.Add(
                    MatchmakingMetadataLobbyHeartbeatTimestamp,
                    FDateTime::UtcNow().ToUnixTimestamp());

                if (!Pins->OnlineLobby->UpdateLobby(
                        *Task->Request.UserId,
                        *Task->MatchmakingLobbyId,
                        *Transaction,
                        FOnLobbyOperationComplete::CreateLambda(
                            [MatchmakingLobbyId = Task->MatchmakingLobbyId->ToString(),
                             TaskId = Task->TaskId,
                             UpdateTaskWk = TWeakPtr<FMatchmakingEngineTask>(
                                 Task)](const FOnlineError &Error, const FUniqueNetId &UserId) {
                                if (!Error.bSucceeded)
                                {
                                    if (Error.ErrorCode.EndsWith(".timed_out") ||
                                        Error.ErrorCode.EndsWith(".already_pending"))
                                    {
                                        // Time out errors during heartbeating don't matter, because we'll
                                        // heartbeat again soon anyway.
                                        return;
                                    }

                                    if (auto UpdateTask = UpdateTaskWk.Pin())
                                    {
                                        if (!UpdateTask->MatchmakingFollowRequestLobbyId
                                                 .IsValid() /* Are we no longer a host? */
                                            && Error.GetErrorResult() == EOnlineErrorResult::FailExtended &&
                                            Error.GetErrorCode().EndsWith(".not_found"))
                                        {
                                            // We aren't the host any more, so we probably deleted the host
                                            // lobby while trying to heartbeat, and then the heartbeat failed
                                            // because the delete happened first. Ignore this error.
                                            return;
                                        }

                                        UE_LOG(
                                            LogMatchmaking,
                                            Warning,
                                            TEXT("Task %s: Failed to heartbeat matchmaking lobby %s: %s"),
                                            *TaskId,
                                            *MatchmakingLobbyId,
                                            *Error.ToLogString());
                                    }
                                    else
                                    {
                                        // The task doesn't exist any more (it's completed), so ignore any
                                        // error here.
                                        return;
                                    }
                                }
                                else
                                {
                                    if (auto UpdateTask = UpdateTaskWk.Pin())
                                    {
                                        UE_LOG(
                                            LogMatchmaking,
                                            VeryVerbose,
                                            TEXT("Task %s: Successfully heartbeated matchmaking lobby %s"),
                                            *UpdateTask->TaskId,
                                            *UpdateTask->MatchmakingLobbyId->ToString());
                                    }
                                }
                            })))
                {
                    UE_LOG(
                        LogMatchmaking,
                        Warning,
                        TEXT("Task %s: Failed to heartbeat matchmaking lobby %s"),
                        *Task->TaskId,
                        *Task->MatchmakingLobbyId->ToString());
                }
            }
        }
    }

    Task->HeartbeatDelegateHandle = FTSTicker::GetCoreTicker().AddTicker(
        FTickerDelegate::CreateSP(
            this,
            &FMatchmakingEngineImpl::OnHostSendHeartbeat,
            TWeakPtr<FMatchmakingEngineTask>(Task)),
        MatchmakingTimingHeartbeatFrequency);
    return false;
}

void FMatchmakingEngineImpl::PrepareForTaskComplete(
    const FString &TaskId,
    const struct FMatchmakingEngineRequest &Request,
    FMatchmakingEngineResponse Response,
    FMatchmakingEngineRequestHandle Handle,
    FMatchmakingEngineRequestComplete OriginalOnComplete)
{
    // Stop the heartbeat handler.
    FTSTicker::GetCoreTicker().RemoveTicker(this->QueuedTasks[Handle]->HeartbeatDelegateHandle);

    // Immediately de-register events.
    for (const auto &EH : this->RegisteredEventHandlers)
    {
        EH->UnregisterEvents(*this, *this->QueuedTasks[Handle]);
    }

    // At the time this callback fires, there may still be references to the task in the call stack (e.g. parameters and
    // locals that may be TSharedPtr<> references to the task). For correctness, when we remove the task from the queue,
    // we want to ensure that the memory associated with it will be freed via an IsUnique call. Therefore, we delay
    // removing and completing the task by scheduling it to happen on the next frame, releasing any other references
    // that might still exist in the call stack.
    this->OnSchedule(
        0.0f,
        FMatchmakingEngineTaskScheduleCallback::CreateSP(
            this,
            &FMatchmakingEngineImpl::ExecuteTaskComplete,
            TaskId,
            Request,
            MoveTemp(Response),
            Handle,
            MoveTemp(OriginalOnComplete)));
}

void FMatchmakingEngineImpl::ExecuteTaskComplete(
    FString TaskId,
    FMatchmakingEngineRequest Request,
    FMatchmakingEngineResponse Response,
    FMatchmakingEngineRequestHandle Handle,
    FMatchmakingEngineRequestComplete OriginalOnComplete)
{
    TSharedPtr<FMatchmakingEngineTask> Task = this->QueuedTasks[Handle];
    checkf(Task.IsValid(), TEXT("The matchmaking task was not valid at the time of completion."));
    Task->StepStates.Empty();
    this->QueuedTasks.Remove(Handle);
    EMIT_TASK_COUNTER_TRACE(Task);
    checkf(Task.IsUnique(), TEXT("The matchmaking task did not have a single reference left at completion."));
    OriginalOnComplete.ExecuteIfBound(TaskId, Request, MoveTemp(Response));
}

void FMatchmakingEngineImpl::PrepareForTaskCancelled(
    const FString &TaskId,
    const struct FMatchmakingEngineRequest &Request,
    FMatchmakingEngineRequestHandle Handle,
    FMatchmakingEngineRequestCancelled OriginalOnCancelled)
{
    FTSTicker::GetCoreTicker().RemoveTicker(this->QueuedTasks[Handle]->HeartbeatDelegateHandle);
    for (const auto &EH : this->RegisteredEventHandlers)
    {
        EH->UnregisterEvents(*this, *this->QueuedTasks[Handle]);
    }
    this->OnSchedule(
        0.0f,
        FMatchmakingEngineTaskScheduleCallback::CreateSP(
            this,
            &FMatchmakingEngineImpl::ExecuteTaskCancelled,
            TaskId,
            Request,
            Handle,
            MoveTemp(OriginalOnCancelled)));
}

void FMatchmakingEngineImpl::ExecuteTaskCancelled(
    FString TaskId,
    FMatchmakingEngineRequest Request,
    FMatchmakingEngineRequestHandle Handle,
    FMatchmakingEngineRequestCancelled OriginalOnCancelled)
{
    auto Task = this->QueuedTasks[Handle];
    Task->StepStates.Empty();
    this->QueuedTasks.Remove(Handle);
    EMIT_TASK_COUNTER_TRACE(Task);
    check(Task.IsUnique());
    OriginalOnCancelled.ExecuteIfBound(TaskId, Request);
}

void FMatchmakingEngineImpl::PrepareForTaskError(
    const FString &TaskId,
    const struct FMatchmakingEngineRequest &Request,
    const FOnlineError &Error,
    FMatchmakingEngineRequestHandle Handle,
    FMatchmakingEngineRequestError OriginalOnError)
{
    FTSTicker::GetCoreTicker().RemoveTicker(this->QueuedTasks[Handle]->HeartbeatDelegateHandle);
    for (const auto &EH : this->RegisteredEventHandlers)
    {
        EH->UnregisterEvents(*this, *this->QueuedTasks[Handle]);
    }
    this->OnSchedule(
        0.0f,
        FMatchmakingEngineTaskScheduleCallback::CreateSP(
            this,
            &FMatchmakingEngineImpl::ExecuteTaskError,
            TaskId,
            Request,
            (FOnlineError)Error,
            Handle,
            MoveTemp(OriginalOnError)));
}

void FMatchmakingEngineImpl::ExecuteTaskError(
    FString TaskId,
    FMatchmakingEngineRequest Request,
    FOnlineError Error,
    FMatchmakingEngineRequestHandle Handle,
    FMatchmakingEngineRequestError OriginalOnError)
{
    auto Task = this->QueuedTasks[Handle];
    Task->StepStates.Empty();
    this->QueuedTasks.Remove(Handle);
    EMIT_TASK_COUNTER_TRACE(Task);
    check(Task.IsUnique());
    OriginalOnError.ExecuteIfBound(TaskId, Request, Error);
}

void FMatchmakingEngineImpl::Tick(float DeltaTime)
{
    this->ElapsedTime += DeltaTime;
    TArray<FScheduledCallback> CallbacksToFire;
    for (int i = this->ScheduledCallbacks.Num() - 1; i >= 0; i--)
    {
        if (this->ElapsedTime >= this->ScheduledCallbacks[i].TargetElapsedTime)
        {
            CallbacksToFire.Insert(this->ScheduledCallbacks[i], 0);
            this->ScheduledCallbacks.RemoveAt(i);
        }
    }
    for (const auto &Cb : CallbacksToFire)
    {
        if (Cb.Callback.IsBound())
        {
            Cb.Callback.Execute();
        }
    }
}

FMatchmakingEngineRequestHandle FMatchmakingEngineImpl::Enqueue(const FMatchmakingEngineRequest Request)
{
    FMatchmakingEngineRequestHandle Handle = this->NextHandle++;

    FMatchmakingEngineRequest ProxiedRequest = Request;
    ProxiedRequest.CompletionBehaviour.OnComplete = FMatchmakingEngineRequestComplete::CreateSP(
        this,
        &FMatchmakingEngineImpl::PrepareForTaskComplete,
        Handle,
        Request.CompletionBehaviour.OnComplete);
    ProxiedRequest.OnCancelled = FMatchmakingEngineRequestCancelled::CreateSP(
        this,
        &FMatchmakingEngineImpl::PrepareForTaskCancelled,
        Handle,
        Request.OnCancelled);
    ProxiedRequest.OnError = FMatchmakingEngineRequestError::CreateSP(
        this,
        &FMatchmakingEngineImpl::PrepareForTaskError,
        Handle,
        Request.OnError);

    TSharedPtr<FMatchmakingEngineTask> Task = MakeShared<FMatchmakingEngineTask>(ProxiedRequest, Handle);
    Task->Schedule = FMatchmakingEngineTaskSchedule::CreateSP(this, &FMatchmakingEngineImpl::OnSchedule);
    Task->Unschedule = FMatchmakingEngineTaskUnschedule::CreateSP(this, &FMatchmakingEngineImpl::OnUnschedule);
    this->QueuedTasks.Add(Handle, Task);
    for (const auto &EH : this->RegisteredEventHandlers)
    {
        EH->RegisterEvents(*this, *Task);
    }

    auto InitialStep = this->GetRegisteredStep<FMatchmakingAlgorithmStepValidateRequest>(MMSTEP_INITIAL);
    Task->CurrentStep = InitialStep->GetName();
    Task->StepStates.Add(Task->CurrentStep, InitialStep->CreateNewState());
    Task->HeartbeatDelegateHandle = FTSTicker::GetCoreTicker().AddTicker(
        FTickerDelegate::CreateSP(
            this,
            &FMatchmakingEngineImpl::OnHostSendHeartbeat,
            TWeakPtr<FMatchmakingEngineTask>(Task)),
        MatchmakingTimingHeartbeatFrequency);
    InitialStep->Start(Task.ToSharedRef());

    return Handle;
}

void FMatchmakingEngineImpl::Cancel(FMatchmakingEngineRequestHandle Handle)
{
    if (!this->QueuedTasks.Contains(Handle))
    {
        // No such task is queued.
        return;
    }

    auto Task = this->QueuedTasks[Handle];

    auto StepName = Task->CurrentStep;
    auto Step = this->GetRegisteredStep<IMatchmakingAlgorithmStep<IMatchmakingAlgorithmStepState>>(StepName);

    if (Step->CanHandleInterrupt(Task.ToSharedRef(), MMINTERRUPT_HAS_SPECIAL_HANDLING_OF_ABANDONMENT))
    {
        if (!Step->HandleInterrupt(Task.ToSharedRef(), MMINTERRUPT_ABANDON_CANCEL))
        {
            Step->MoveToStep(*Task, MMSTEP_ABANDON_START, EMatchmakingAbandonmentType::Cancel);
        }
    }
    else
    {
        Step->MoveToStep(*Task, MMSTEP_ABANDON_START, EMatchmakingAbandonmentType::Cancel);
    }
}

#endif

REDPOINT_EOS_CODE_GUARD_END()