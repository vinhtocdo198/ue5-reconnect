// Copyright June Rhodes. All Rights Reserved.

#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepCompleteMatchAsClient.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepCompleteMatchFireEvent.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepWaitForSessionIdOrConnectionString.h"
#include "RedpointMatchmaking/MatchmakingConstants.h"
#include "RedpointMatchmaking/MatchmakingLog.h"
#include "RedpointMatchmaking/MatchmakingSerialization.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

TSharedPtr<FMatchmakingAlgorithmStepStateCompleteMatchAsClient> FMatchmakingAlgorithmStepCompleteMatchAsClient::
    CreateNewState()
{
    return MakeShared<FMatchmakingAlgorithmStepStateCompleteMatchAsClient>();
}

void FMatchmakingAlgorithmStepCompleteMatchAsClient::OnMarkedAsFinalized(
    const FOnlineError &Error,
    const FUniqueNetId &UserId,
    TWeakPtr<FMatchmakingEngineTask> TaskWk,
    TSharedRef<FMatchmakingEngineResponse> Result)
{
    auto Task = TaskWk.Pin();
    if (!Task.IsValid())
    {
        return;
    }

    if (!Task->CurrentStep.IsEqual(MMSTEP_COMPLETE_MATCH_AS_CLIENT))
    {
        return;
    }

    if (!Error.bSucceeded)
    {
        if (Error.ErrorCode.EndsWith(".timed_out") || Error.ErrorCode.EndsWith(".already_pending"))
        {
            if (auto Pins = Task->PinAndContinue())
            {
                auto Txn = Pins->OnlineLobby->MakeUpdateLobbyMemberTransaction(
                    *Task->Request.UserId,
                    *Task->MatchmakingLobbyId,
                    *Task->Request.UserId);
                Txn->SetMetadata.Add(MatchmakingMemberMetadataFinalized, true);
                if (!Pins->OnlineLobby->UpdateMemberSelf(
                        *Task->Request.UserId,
                        *Task->MatchmakingLobbyId,
                        *Txn,
                        FOnLobbyOperationComplete::CreateSP(
                            this,
                            &FMatchmakingAlgorithmStepCompleteMatchAsClient::OnMarkedAsFinalized,
                            TWeakPtr<FMatchmakingEngineTask>(Task),
                            Result)))
                {
                    this->RestartableError(
                        *Task,
                        TEXT("Unable to tell the host that we've finalized on the client during retry for timed_out!"));
                }
                return;
            }
        }

        this->RestartableError(
            *Task,
            FString::Printf(
                TEXT("Unable to tell the host that we've finalized on the client: %s"),
                *Error.ToLogString()));
        return;
    }

    UE_LOG(LogMatchmaking, Verbose, TEXT("%sFinished matchmaking on client"), *this->GetLogPrefix(*Task));

    FMatchmakingSerialization::DumpTeamArrayState(Result->Teams, TEXT("CompleteMatchAsClient"));

    switch (Task->Request.CompletionBehaviour.Type)
    {
    case EMatchmakingEngineCompletionBehaviourType::ReturnResults:
        Task->StoredResponse = Result;
        this->MoveToStep(*Task, MMSTEP_COMPLETE_MATCH_FIRE_EVENT);
        break;
    case EMatchmakingEngineCompletionBehaviourType::StartListenServerWithMap:
    case EMatchmakingEngineCompletionBehaviourType::FindDedicatedServer:
        Task->Request.CompletionBehaviour.OnResultsReady.ExecuteIfBound(Task->TaskId, Task->Request, *Result);
        Task->StoredResponse = Result;
        this->MoveToStep(*Task, MMSTEP_WAIT_FOR_SESSION_ID_OR_CONNECTION_STRING);
        break;
    default:
        checkf(false, TEXT("Unsupported CompletionBehaviour.Type"));
        break;
    }
}

void FMatchmakingAlgorithmStepCompleteMatchAsClient::Start(TSharedRef<FMatchmakingEngineTask> Task)
{
    if (auto Pins = Task->PinAndContinue())
    {
        UE_LOG(LogMatchmaking, Verbose, TEXT("%sFinishing up on client..."), *this->GetLogPrefix(*Task));

        auto Result = MakeShared<FMatchmakingEngineResponse>();
        Result->Teams = FMatchmakingSerialization::DeserializeResultFromLobbyData(
            Pins->OnlineLobby.ToSharedRef(),
            Pins->OnlineIdentity.ToSharedRef(),
            *Task->Request.UserId,
            *Task->MatchmakingLobbyId,
            MatchmakingMetadataResult);

        UE_LOG(
            LogMatchmaking,
            Verbose,
            TEXT("%sMarking matchmaking as finished on client..."),
            *this->GetLogPrefix(*Task));

        auto Txn = Pins->OnlineLobby->MakeUpdateLobbyMemberTransaction(
            *Task->Request.UserId,
            *Task->MatchmakingLobbyId,
            *Task->Request.UserId);
        Txn->SetMetadata.Add(MatchmakingMemberMetadataFinalized, true);

        if (!Pins->OnlineLobby->UpdateMemberSelf(
                *Task->Request.UserId,
                *Task->MatchmakingLobbyId,
                *Txn,
                FOnLobbyOperationComplete::CreateSP(
                    this,
                    &FMatchmakingAlgorithmStepCompleteMatchAsClient::OnMarkedAsFinalized,
                    TWeakPtr<FMatchmakingEngineTask>(Task),
                    Result)))
        {
            this->RestartableError(*Task, TEXT("Unable to tell the host that we've finalized on the client!"));
        }
    }
}

#endif

REDPOINT_EOS_CODE_GUARD_END()