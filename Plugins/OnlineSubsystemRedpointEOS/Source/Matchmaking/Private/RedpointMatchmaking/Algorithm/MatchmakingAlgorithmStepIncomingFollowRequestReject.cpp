// Copyright June Rhodes. All Rights Reserved.

#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepIncomingFollowRequestReject.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "../MatchmakingFollowRequest.h"
#include "../MatchmakingFollowResponse.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepSearching.h"
#include "RedpointMatchmaking/MatchmakingConstants.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

TSharedPtr<FMatchmakingAlgorithmStepStateIncomingFollowRequestReject>
FMatchmakingAlgorithmStepIncomingFollowRequestReject::CreateNewState()
{
    return MakeShared<FMatchmakingAlgorithmStepStateIncomingFollowRequestReject>();
}

void FMatchmakingAlgorithmStepIncomingFollowRequestReject::OnFollowResponseSent(
    const FOnlineError &Error,
    const FUniqueNetId &UserId,
    TWeakPtr<FMatchmakingEngineTask> TaskWk)
{
    auto Task = TaskWk.Pin();
    if (!Task.IsValid())
    {
        return;
    }

    if (!Task->CurrentStep.IsEqual(MMSTEP_INCOMING_FOLLOW_REQUEST_REJECT))
    {
        return;
    }

    if (!Error.bSucceeded)
    {
        if (Error.ErrorCode.EndsWith(".timed_out"))
        {
            // It's fine if we timeout here, because the requester will time us out
            // if we don't send a response back fast enough.
        }
    }

    EMIT_TASK_COUNTER_TRACE(Task);
    this->MoveToStep(*Task, MMSTEP_SEARCHING);
}

void FMatchmakingAlgorithmStepIncomingFollowRequestReject::Start(
    TSharedRef<FMatchmakingEngineTask> Task,
    TSharedPtr<FOnlineLobbyId> TargetLobbyId,
    TSharedRef<const FUniqueNetId> TargetMemberId,
    int32 TargetRequestId)
{
    if (auto Pins = Task->PinAndContinue())
    {
        auto Txn = Pins->OnlineLobby->MakeUpdateLobbyTransaction(
            *Task->Request.UserId,
            *Task->MatchmakingFollowRequestLobbyId);
        Txn->SetMetadata.Add(
            FString::Printf(TEXT("%s.%s"), MatchmakingMemberMetadataFollowResponse, *TargetMemberId->ToString()),
            FMatchmakingFollowResponse::SerializeFollowResponse(TargetRequestId, false));

        if (!Pins->OnlineLobby->UpdateLobby(
                *Task->Request.UserId,
                *Task->MatchmakingFollowRequestLobbyId,
                *Txn,
                FOnLobbyOperationComplete::CreateSP(
                    this,
                    &FMatchmakingAlgorithmStepIncomingFollowRequestReject::OnFollowResponseSent,
                    TWeakPtr<FMatchmakingEngineTask>(Task))))
        {
            EMIT_TASK_COUNTER_TRACE(Task);
            this->MoveToStep(*Task, MMSTEP_SEARCHING);
        }
    }
}

#endif

REDPOINT_EOS_CODE_GUARD_END()