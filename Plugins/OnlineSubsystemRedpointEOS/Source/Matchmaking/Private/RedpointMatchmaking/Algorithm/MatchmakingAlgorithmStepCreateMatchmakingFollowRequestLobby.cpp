// Copyright June Rhodes. All Rights Reserved.

#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepCreateMatchmakingFollowRequestLobby.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepCreateMatchmakingLobby.h"
#include "RedpointMatchmaking/MatchmakingConstants.h"
#include "RedpointMatchmaking/MatchmakingLog.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

TSharedPtr<FMatchmakingAlgorithmStepStateCreateMatchmakingFollowRequestLobby>
FMatchmakingAlgorithmStepCreateMatchmakingFollowRequestLobby::CreateNewState()
{
    return MakeShared<FMatchmakingAlgorithmStepStateCreateMatchmakingFollowRequestLobby>();
}

void FMatchmakingAlgorithmStepCreateMatchmakingFollowRequestLobby::OnLobbyCreated(
    const FOnlineError &Error,
    const FUniqueNetId &UserId,
    const TSharedPtr<class FOnlineLobby> &Lobby,
    TWeakPtr<FMatchmakingEngineTask> TaskWk)
{
    auto Task = TaskWk.Pin();
    if (!Task.IsValid())
    {
        return;
    }

    if (!Task->CurrentStep.IsEqual(MMSTEP_CREATE_MATCHMAKING_FOLLOW_REQUEST_LOBBY))
    {
        return;
    }

    if (!Error.bSucceeded)
    {
        if (Error.ErrorCode.EndsWith(".timed_out") || Error.ErrorCode.EndsWith(".already_pending"))
        {
            // Retry on timeout. It's safe to just call Start() again for this implementation.
            this->Start(Task.ToSharedRef());
            return;
        }

        this->RestartableError(
            *Task,
            FString::Printf(TEXT("Unable to create lobby for matchmaking requests: %s"), *Error.ToLogString()));
        return;
    }

    if (auto Pins = Task->PinAndContinue())
    {
        Task->MatchmakingFollowRequestLobbyId = Lobby->Id;

        UE_LOG(LogMatchmaking, Verbose, TEXT("%sCreated matchmaking follow request lobby"), *this->GetLogPrefix(*Task));

        EMIT_TASK_COUNTER_TRACE(Task);
        this->MoveToStep(*Task, MMSTEP_CREATE_MATCHMAKING_LOBBY);
    }
}

void FMatchmakingAlgorithmStepCreateMatchmakingFollowRequestLobby::Start(TSharedRef<FMatchmakingEngineTask> Task)
{
    if (auto Pins = Task->PinAndContinue())
    {
        UE_LOG(
            LogMatchmaking,
            Verbose,
            TEXT("%sCreating a matchmaking follow request lobby"),
            *this->GetLogPrefix(*Task));

        auto Transaction = Pins->OnlineLobby->MakeCreateLobbyTransaction(*Task->Request.UserId);
        Transaction->Capacity = MatchmakingFollowRequestLobbyCapacity;
        Transaction->Locked = false;
        Transaction->Public = true;
        Transaction->SetMetadata.Add(
            MatchmakingMetadataLobbyType,
            FVariantData(MatchmakingLobbyTypeMatchFollowRequests));

        if (!Pins->OnlineLobby->CreateLobby(
                *Task->Request.UserId,
                *Transaction,
                FOnLobbyCreateOrConnectComplete::CreateSP(
                    this,
                    &FMatchmakingAlgorithmStepCreateMatchmakingFollowRequestLobby::OnLobbyCreated,
                    TWeakPtr<FMatchmakingEngineTask>(Task))))
        {
            this->RestartableError(*Task, TEXT("Unable to create lobby for matchmaking requests"));
            return;
        }
    }
}

#endif

REDPOINT_EOS_CODE_GUARD_END()