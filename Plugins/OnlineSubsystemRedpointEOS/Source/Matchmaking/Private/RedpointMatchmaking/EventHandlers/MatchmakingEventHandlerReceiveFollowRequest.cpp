// Copyright June Rhodes. All Rights Reserved.

#include "MatchmakingEventHandlerReceiveFollowRequest.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "../Algorithm/MatchmakingAlgorithmStep.h"
#include "../Algorithm/MatchmakingAlgorithmStepIncomingFollowRequestReject.h"
#include "../Algorithm/MatchmakingAlgorithmStepIncomingFollowRequestVerify.h"
#include "../MatchmakingFollowRequest.h"
#include "../MatchmakingFollowResponse.h"
#include "../MatchmakingStateHash.h"
#include "RedpointMatchmaking/MatchmakingConstants.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMHANDLER_RECEIVE_FOLLOW_REQUEST FName("EventHandlerReceiveFollowRequest")

void FMatchmakingEventHandlerReceiveFollowRequest::OnFollowRequestTimerElapsed(
    TSharedRef<FMatchmakingEngineImpl> Engine,
    TWeakPtr<FMatchmakingEngineTask> TaskWk)
{
    auto Task = TaskWk.Pin();
    if (!Task.IsValid())
    {
        return;
    }

    if (!Task->MatchmakingFollowRequestLobbyId.IsValid() || !Task->MatchmakingLobbyId.IsValid())
    {
        // We're no longer the host and our follow request lobby has disappeared, so ignore this callback.
        return;
    }

    EMIT_TASK_COUNTER_TRACE(Task);

    if (Task->IncomingFollowRequestsForConsideration.Num() == 0)
    {
        Task->IncomingFollowRequestsForConsideration.Empty();
        Task->IncomingFollowRequestDelayTimerHandle.Reset();
        return;
    }

    // Generate a state hash for our own lobby.
    FString StateHash;
    if (auto Pins = Task->PinAndContinue())
    {
        StateHash = FMatchmakingStateHash::ComputeStateHashForLobby(
            *Pins->OnlineLobby,
            *Task->Request.UserId,
            *Task->MatchmakingLobbyId);
    }

    // Sort the incoming follow requests by oldest lobby first.
    Task->IncomingFollowRequestsForConsideration.StableSort([](const FMatchmakingEngineTaskIncomingFollowRequest &LHS,
                                                               const FMatchmakingEngineTaskIncomingFollowRequest &RHS) {
        return LHS.TargetLobbyCreatedUnixTimestamp < RHS.TargetLobbyCreatedUnixTimestamp;
    });

    for (int32 i = 0; i < Task->IncomingFollowRequestsForConsideration.Num(); i++)
    {
        UE_LOG(
            LogMatchmaking,
            Verbose,
            TEXT("%sIncoming follow request for consideration #%d: (From) %s (Target) %s (Created) %lld"),
            *this->GetLogPrefix(*Task, MMHANDLER_RECEIVE_FOLLOW_REQUEST),
            i,
            *Task->IncomingFollowRequestsForConsideration[i].FromMemberId->ToString(),
            *Task->IncomingFollowRequestsForConsideration[i].TargetLobbyId->ToString(),
            Task->IncomingFollowRequestsForConsideration[i].TargetLobbyCreatedUnixTimestamp);
    }

    // Determine the selected request.
    TSharedPtr<FMatchmakingEngineTaskIncomingFollowRequest> SelectedRequest;
    for (int32 i = 0; i < Task->IncomingFollowRequestsForConsideration.Num(); i++)
    {
        if (Task->IncomingFollowRequestsForConsideration[i].TargetStateHash != StateHash)
        {
            // Excluded because the state hash doesn't match.
            UE_LOG(
                LogMatchmaking,
                Verbose,
                TEXT("%sIgnoring follow request from %s because the provided state hash is %s but the locally computed "
                     "value is %s"),
                *this->GetLogPrefix(*Task, MMHANDLER_RECEIVE_FOLLOW_REQUEST),
                *Task->IncomingFollowRequestsForConsideration[i].FromMemberId->ToString(),
                *Task->IncomingFollowRequestsForConsideration[i].TargetStateHash,
                *StateHash);
            continue;
        }

        SelectedRequest =
            MakeShared<FMatchmakingEngineTaskIncomingFollowRequest>(Task->IncomingFollowRequestsForConsideration[i]);
        break;
    }

    // Reject all non-selected requests.
    if (auto Pins = Task->PinAndContinue())
    {
        for (int32 i = 0; i < Task->IncomingFollowRequestsForConsideration.Num(); i++)
        {
            const auto &RejectedRequest = Task->IncomingFollowRequestsForConsideration[i];

            if (SelectedRequest.IsValid())
            {
                if (*RejectedRequest.FromMemberId == *SelectedRequest->FromMemberId)
                {
                    // In case the same user sent multiple requests, and multiple requests got queued up.
                    continue;
                }
            }

            Task->RejectedRequestsSinceLastSearch++;

            auto Txn = Pins->OnlineLobby->MakeUpdateLobbyTransaction(
                *Task->Request.UserId,
                *Task->MatchmakingFollowRequestLobbyId);
            Txn->SetMetadata.Add(
                FString::Printf(
                    TEXT("%s.%s"),
                    MatchmakingMemberMetadataFollowResponse,
                    *RejectedRequest.FromMemberId->ToString()),
                FMatchmakingFollowResponse::SerializeFollowResponse(RejectedRequest.TargetRequestId, false));
            if (!Pins->OnlineLobby->UpdateLobby(
                    *Task->Request.UserId,
                    *Task->MatchmakingFollowRequestLobbyId,
                    *Txn,
                    FOnLobbyOperationComplete()))
            {
                UE_LOG(
                    LogMatchmaking,
                    Warning,
                    TEXT("%sCan't send rejection follow response to user %s"),
                    *this->GetLogPrefix(*Task, MMHANDLER_RECEIVE_FOLLOW_REQUEST),
                    *RejectedRequest.FromMemberId->ToString());
            }
        }

        // Only run acceptance logic if we actually selected a request.
        if (SelectedRequest.IsValid())
        {
            auto Step =
                Engine->GetRegisteredStep<IMatchmakingAlgorithmStep<IMatchmakingAlgorithmStepState>>(Task->CurrentStep);
            if (!Step->CanHandleInterrupt(Task.ToSharedRef(), MMINTERRUPT_WANT_TO_ACCEPT_FOLLOW_REQUEST))
            {
                // We can't accept the follow request, because the current step does not support follow
                // requests.
                UE_LOG(
                    LogMatchmaking,
                    Verbose,
                    TEXT("%sRejecting follow request from %s because the current step (%s) does not allow "
                         "follow request acceptance"),
                    *this->GetLogPrefix(*Task, MMHANDLER_RECEIVE_FOLLOW_REQUEST),
                    *SelectedRequest->FromMemberId->ToString(),
                    *Task->CurrentStep.ToString());

                Task->RejectedRequestsSinceLastSearch++;

                auto Txn = Pins->OnlineLobby->MakeUpdateLobbyTransaction(
                    *Task->Request.UserId,
                    *Task->MatchmakingFollowRequestLobbyId);
                Txn->SetMetadata.Add(
                    FString::Printf(
                        TEXT("%s.%s"),
                        MatchmakingMemberMetadataFollowResponse,
                        *SelectedRequest->FromMemberId->ToString()),
                    FMatchmakingFollowResponse::SerializeFollowResponse(SelectedRequest->TargetRequestId, false));
                if (!Pins->OnlineLobby->UpdateLobby(
                        *Task->Request.UserId,
                        *Task->MatchmakingFollowRequestLobbyId,
                        *Txn,
                        FOnLobbyOperationComplete()))
                {
                    UE_LOG(
                        LogMatchmaking,
                        Warning,
                        TEXT("%sCan't send rejection follow response to user"),
                        *this->GetLogPrefix(*Task, MMHANDLER_RECEIVE_FOLLOW_REQUEST));
                }

                // We do not fall through to bAcceptFollowRequest because that will invoke MoveToStep and
                // the current step can't handle the interrupt at all.
                Task->IncomingFollowRequestsForConsideration.Empty();
                Task->IncomingFollowRequestDelayTimerHandle.Reset();
                return;
            }

            // If we accept, we must verify connection before we send the Accepted response (even though we
            // have already interrupted our step). Because we interrupted our step, we don't have to worry
            // about anything else happen until we finish verification. If we can't verify, we move back to
            // searching. If we rejected above, we send the response immediately.
            if (Step->HandleInterrupt(Task.ToSharedRef(), MMINTERRUPT_WANT_TO_ACCEPT_FOLLOW_REQUEST))
            {
                UE_LOG(
                    LogMatchmaking,
                    Verbose,
                    TEXT("%sAllowed to accept follow request from %s, verifying first"),
                    *this->GetLogPrefix(*Task, MMHANDLER_RECEIVE_FOLLOW_REQUEST),
                    *SelectedRequest->FromMemberId->ToString());

                EMIT_TASK_COUNTER_TRACE(Task);
                Step->MoveToStep(
                    *Task,
                    MMSTEP_INCOMING_FOLLOW_REQUEST_VERIFY,
                    SelectedRequest->TargetLobbyId,
                    SelectedRequest->FromMemberId,
                    SelectedRequest->TargetRequestId);
                EMIT_TASK_COUNTER_TRACE(Task);
            }
            else
            {
                UE_LOG(
                    LogMatchmaking,
                    Verbose,
                    TEXT("%sRejecting follow request from %s because the current step (%s) rejected our "
                         "request "
                         "to accept"),
                    *this->GetLogPrefix(*Task, MMHANDLER_RECEIVE_FOLLOW_REQUEST),
                    *SelectedRequest->FromMemberId->ToString(),
                    *Task->CurrentStep.ToString());

                Task->RejectedRequestsSinceLastSearch++;

                EMIT_TASK_COUNTER_TRACE(Task);
                Step->MoveToStep(
                    *Task,
                    MMSTEP_INCOMING_FOLLOW_REQUEST_REJECT,
                    SelectedRequest->TargetLobbyId,
                    SelectedRequest->FromMemberId,
                    SelectedRequest->TargetRequestId);
                EMIT_TASK_COUNTER_TRACE(Task);
            }
        }
    }

    Task->IncomingFollowRequestsForConsideration.Empty();
    Task->IncomingFollowRequestDelayTimerHandle.Reset();
    return;
}

void FMatchmakingEventHandlerReceiveFollowRequest::OnMemberUpdate(
    const FUniqueNetId &UserId,
    const FOnlineLobbyId &LobbyId,
    const FUniqueNetId &MemberId,
    TSharedRef<FMatchmakingEngineImpl> Engine,
    TWeakPtr<FMatchmakingEngineTask> TaskWk)
{
    auto Task = TaskWk.Pin();
    check(Task.IsValid());

    EMIT_TASK_COUNTER_TRACE(Task);

    if (Task->MatchmakingFollowRequestLobbyId.IsValid() && *Task->MatchmakingFollowRequestLobbyId == LobbyId &&
        *Task->Request.UserId == UserId)
    {
        if (auto Pins = Task->PinAndContinue())
        {
            FVariantData FollowRequestData;
            if (Pins->OnlineLobby->GetMemberMetadataValue(
                    UserId,
                    LobbyId,
                    MemberId,
                    MatchmakingMemberMetadataFollowRequest,
                    FollowRequestData))
            {
                TSharedPtr<const FOnlineLobbyId> TargetLobbyId;
                int32 TargetRequestId;
                int64 TargetLobbyCreatedUnixTimestamp;
                FString TargetStateHash;
                if (FMatchmakingFollowRequest::TryDeserializeFollowRequest(
                        Pins->OnlineLobby,
                        FollowRequestData,
                        TargetLobbyId,
                        TargetRequestId,
                        TargetLobbyCreatedUnixTimestamp,
                        TargetStateHash))
                {
                    // Make sure we don't handle a request more than once.
                    if (Task->EventData.Contains(FName(*TargetLobbyId->ToString())))
                    {
                        int32 CurrentRequestId;
                        Task->EventData[FName(*TargetLobbyId->ToString())].GetValue(CurrentRequestId);
                        if (TargetRequestId == CurrentRequestId)
                        {
                            return;
                        }
                    }
                    Task->EventData.Add(FName(*TargetLobbyId->ToString()), TargetRequestId);

                    UE_LOG(
                        LogMatchmaking,
                        Verbose,
                        TEXT("%sReceived follow request from %s, adding to queue"),
                        *this->GetLogPrefix(*Task, MMHANDLER_RECEIVE_FOLLOW_REQUEST),
                        *MemberId.ToString());

                    Task->IncomingFollowRequestsForConsideration.Add(FMatchmakingEngineTaskIncomingFollowRequest(
                        MemberId.AsShared(),
                        TargetLobbyId.ToSharedRef(),
                        TargetRequestId,
                        TargetLobbyCreatedUnixTimestamp,
                        TargetStateHash));

                    // Because of the way instant scheduling works during unit tests, we need to ensure that
                    // OnFollowRequestTimerElapsed fires faster than the follow request timeout.
                    if (Engine->IsInstantScheduling())
                    {
                        this->OnFollowRequestTimerElapsed(MoveTemp(Engine), TaskWk);
                    }
                    else if (!Task->IncomingFollowRequestDelayTimerHandle.IsValid())
                    {
                        Task->IncomingFollowRequestDelayTimerHandle = Engine->OnSchedule(
                            MatchmakingTimingIncomingRequestQueueDelay,
                            FMatchmakingEngineTaskScheduleCallback::CreateSP(
                                this,
                                &FMatchmakingEventHandlerReceiveFollowRequest::OnFollowRequestTimerElapsed,
                                MoveTemp(Engine),
                                TaskWk));
                    }
                }
            }
        }
    }

    EMIT_TASK_COUNTER_TRACE(Task);
}

void FMatchmakingEventHandlerReceiveFollowRequest::RegisterEvents(
    FMatchmakingEngineImpl &Engine,
    FMatchmakingEngineTask &Task)
{
    if (auto Pins = Task.PinAndContinue())
    {
        Task.EventHandlers.Add(
            MMHANDLER_RECEIVE_FOLLOW_REQUEST,
            Pins->OnlineLobby->AddOnMemberUpdateDelegate_Handle(FOnMemberUpdateDelegate::CreateSP(
                this,
                &FMatchmakingEventHandlerReceiveFollowRequest::OnMemberUpdate,
                Engine.AsShared(),
                TWeakPtr<FMatchmakingEngineTask>(Task.AsShared()))));
        EMIT_TASK_COUNTER_TRACE(Task.AsShared());
    }
}

void FMatchmakingEventHandlerReceiveFollowRequest::UnregisterEvents(
    FMatchmakingEngineImpl &Engine,
    FMatchmakingEngineTask &Task)
{
    if (auto Pins = Task.PinAndContinue())
    {
        if (Task.EventHandlers.Contains(MMHANDLER_RECEIVE_FOLLOW_REQUEST))
        {
            Pins->OnlineLobby->ClearOnMemberUpdateDelegate_Handle(Task.EventHandlers[MMHANDLER_RECEIVE_FOLLOW_REQUEST]);
            Task.EventHandlers.Remove(MMHANDLER_RECEIVE_FOLLOW_REQUEST);
            EMIT_TASK_COUNTER_TRACE(Task.AsShared());
        }
    }
}

#endif

REDPOINT_EOS_CODE_GUARD_END()