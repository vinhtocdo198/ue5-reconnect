// Copyright June Rhodes. All Rights Reserved.

#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepIncomingFollowRequestVerify.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepIncomingFollowRequestAccept.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepIncomingFollowRequestReject.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

TSharedPtr<FMatchmakingAlgorithmStepStateIncomingFollowRequestVerify>
FMatchmakingAlgorithmStepIncomingFollowRequestVerify::CreateNewState()
{
    return MakeShared<FMatchmakingAlgorithmStepStateIncomingFollowRequestVerify>();
}

void FMatchmakingAlgorithmStepIncomingFollowRequestVerify::OnConnected(
    const FOnlineError &Error,
    const FUniqueNetId &UserId,
    const TSharedPtr<class FOnlineLobby> &Lobby,
    TWeakPtr<FMatchmakingEngineTask> TaskWk,
    TSharedPtr<FOnlineLobbyId> TargetLobbyId,
    TSharedRef<const FUniqueNetId> TargetMemberId,
    int32 TargetRequestId)
{
    auto Task = TaskWk.Pin();
    if (!Task.IsValid())
    {
        return;
    }

    if (!Task->CurrentStep.IsEqual(MMSTEP_INCOMING_FOLLOW_REQUEST_VERIFY))
    {
        return;
    }

    if (!Error.bSucceeded)
    {
        if (Error.ErrorCode.EndsWith(".timed_out") || Error.ErrorCode.EndsWith(".already_pending"))
        {
            // Just reject if we timed out. It's possible the operation took so long that
            // the requester timed us out on their end anyway, so we can't retry safely here.
        }

        EMIT_TASK_COUNTER_TRACE(Task);
        this->MoveToStep(*Task, MMSTEP_INCOMING_FOLLOW_REQUEST_REJECT, TargetLobbyId, TargetMemberId, TargetRequestId);
        return;
    }

    EMIT_TASK_COUNTER_TRACE(Task);
    this->MoveToStep(*Task, MMSTEP_INCOMING_FOLLOW_REQUEST_ACCEPT, TargetLobbyId, TargetMemberId, TargetRequestId);
}

void FMatchmakingAlgorithmStepIncomingFollowRequestVerify::Start(
    TSharedRef<FMatchmakingEngineTask> Task,
    TSharedPtr<FOnlineLobbyId> TargetLobbyId,
    TSharedRef<const FUniqueNetId> TargetMemberId,
    int32 TargetRequestId)
{
    EMIT_TASK_COUNTER_TRACE(Task);
    if (auto Pins = Task->PinAndContinue())
    {
        if (!Pins->OnlineLobby->ConnectLobby(
                *Task->Request.UserId,
                *TargetLobbyId,
                FOnLobbyCreateOrConnectComplete::CreateSP(
                    this,
                    &FMatchmakingAlgorithmStepIncomingFollowRequestVerify::OnConnected,
                    TWeakPtr<FMatchmakingEngineTask>(Task),
                    TargetLobbyId,
                    TargetMemberId,
                    TargetRequestId)))
        {
            EMIT_TASK_COUNTER_TRACE(Task);
            this->MoveToStep(
                *Task,
                MMSTEP_INCOMING_FOLLOW_REQUEST_REJECT,
                TargetLobbyId,
                TargetMemberId,
                TargetRequestId);
        }
    }
}

#endif

REDPOINT_EOS_CODE_GUARD_END()