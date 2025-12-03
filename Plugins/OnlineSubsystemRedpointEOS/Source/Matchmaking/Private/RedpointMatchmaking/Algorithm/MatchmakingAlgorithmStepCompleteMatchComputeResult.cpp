// Copyright June Rhodes. All Rights Reserved.

#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepCompleteMatchComputeResult.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "../MatchmakingSerialization.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepCompleteMatchFinalizeLobby.h"
#include "RedpointMatchmaking/MatchmakingConstants.h"
#include "RedpointMatchmaking/MatchmakingLog.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

TSharedPtr<FMatchmakingAlgorithmStepStateCompleteMatchComputeResult>
FMatchmakingAlgorithmStepCompleteMatchComputeResult::CreateNewState()
{
    return MakeShared<FMatchmakingAlgorithmStepStateCompleteMatchComputeResult>();
}

void FMatchmakingAlgorithmStepCompleteMatchComputeResult::OnLobbyUpdated(
    const FOnlineError &Error,
    const FUniqueNetId &UserId,
    TWeakPtr<FMatchmakingEngineTask> TaskWk,
    TArray<FMatchmakingEngineResponseTeam> Teams)
{
    auto Task = TaskWk.Pin();
    if (!Task.IsValid())
    {
        return;
    }

    if (!Task->CurrentStep.IsEqual(MMSTEP_COMPLETE_MATCH_COMPUTE_RESULT))
    {
        return;
    }

    if (!Error.bSucceeded)
    {
        if (Error.ErrorCode.EndsWith(".timed_out") || Error.ErrorCode.EndsWith(".already_pending"))
        {
            if (auto Pins = Task->PinAndContinue())
            {
                auto Txn =
                    Pins->OnlineLobby->MakeUpdateLobbyTransaction(*Task->Request.UserId, *Task->MatchmakingLobbyId);
                Txn->SetMetadata.Add(MatchmakingMetadataLobbyHeartbeatTimestamp, FDateTime::UtcNow().ToUnixTimestamp());
                FString Prefix = MatchmakingMetadataResult;
                FMatchmakingSerialization::SerializeResultToTransaction(Teams, Prefix, Txn);
                if (!Pins->OnlineLobby->UpdateLobby(
                        *Task->Request.UserId,
                        *Task->MatchmakingLobbyId,
                        *Txn,
                        FOnLobbyOperationComplete::CreateSP(
                            this,
                            &FMatchmakingAlgorithmStepCompleteMatchComputeResult::OnLobbyUpdated,
                            TWeakPtr<FMatchmakingEngineTask>(Task),
                            Teams)))
                {
                    this->RestartableError(*Task, TEXT("Unable to send results to clients on retry for timed_out"));
                }
                return;
            }
        }

        this->RestartableError(
            *Task,
            FString::Printf(TEXT("Unable to finalize matchmaking lobby: %s"), *Error.ToLogString()));
        return;
    }

    UE_LOG(LogMatchmaking, Verbose, TEXT("%sSent results to clients."), *this->GetLogPrefix(*Task));

    // Wait for clients to process the results.
    EMIT_TASK_COUNTER_TRACE(Task);
    this->MoveToStep(*Task, MMSTEP_COMPLETE_MATCH_FINALIZE_LOBBY);
}

void FMatchmakingAlgorithmStepCompleteMatchComputeResult::Start(TSharedRef<FMatchmakingEngineTask> Task)
{
    if (auto Pins = Task->PinAndContinue())
    {
        UE_LOG(LogMatchmaking, Verbose, TEXT("%sComputing results..."), *this->GetLogPrefix(*Task));

        TArray<FMatchmakingEngineResponseTeam> Teams;

        for (int i = 0; i < Task->Request.HostConfiguration->TeamCapacities.Num(); i++)
        {
            FMatchmakingEngineResponseTeam ResultTeam;
            Teams.Add(ResultTeam);
        }

        for (const auto &UserTeam : Task->TeamManager->GetFinalTeams())
        {
            FMatchmakingEngineResponseTeamSlot Member;
            Member.Type = EMatchmakingEngineResponseTeamSlotType::User;
            Member.UserId = UserTeam.Key;
            Teams[UserTeam.Value].Slots.Add(Member);
        }

        for (int i = 0; i < Task->Request.HostConfiguration->TeamCapacities.Num(); i++)
        {
            int MemberCount = Teams[i].Slots.Num();
            int RemainingSlots = Task->Request.HostConfiguration->TeamCapacities[i] - MemberCount;

            for (int j = 0; j < RemainingSlots; j++)
            {
                FMatchmakingEngineResponseTeamSlot Member;
                Member.Type = Task->Request.HostConfiguration->OnNoCandidates ==
                                      EMatchmakingBehaviourOnNoCandidates::CompleteFillWithAI
                                  ? EMatchmakingEngineResponseTeamSlotType::AI
                                  : EMatchmakingEngineResponseTeamSlotType::Empty;
                Teams[i].Slots.Add(Member);
            }
        }

        FMatchmakingSerialization::DumpTeamArrayState(Teams, TEXT("CompleteMatchComputeResult"));

        UE_LOG(LogMatchmaking, Verbose, TEXT("%sSending results to clients..."), *this->GetLogPrefix(*Task));

        auto Txn = Pins->OnlineLobby->MakeUpdateLobbyTransaction(*Task->Request.UserId, *Task->MatchmakingLobbyId);
        Txn->SetMetadata.Add(MatchmakingMetadataLobbyHeartbeatTimestamp, FDateTime::UtcNow().ToUnixTimestamp());
        FString Prefix = MatchmakingMetadataResult;
        FMatchmakingSerialization::SerializeResultToTransaction(Teams, Prefix, Txn);

        if (!Pins->OnlineLobby->UpdateLobby(
                *Task->Request.UserId,
                *Task->MatchmakingLobbyId,
                *Txn,
                FOnLobbyOperationComplete::CreateSP(
                    this,
                    &FMatchmakingAlgorithmStepCompleteMatchComputeResult::OnLobbyUpdated,
                    TWeakPtr<FMatchmakingEngineTask>(Task),
                    Teams)))
        {
            this->RestartableError(*Task, TEXT("Unable to send results to clients"));
            return;
        }
    }
}

#endif

REDPOINT_EOS_CODE_GUARD_END()