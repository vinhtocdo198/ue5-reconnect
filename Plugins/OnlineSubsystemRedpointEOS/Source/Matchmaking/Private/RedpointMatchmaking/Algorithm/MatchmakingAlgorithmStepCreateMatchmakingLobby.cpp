// Copyright June Rhodes. All Rights Reserved.

#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepCreateMatchmakingLobby.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepNotifyExistingParty.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepWaitingForPlayersToJoin.h"
#include "RedpointMatchmaking/MatchmakingConstants.h"
#include "RedpointMatchmaking/MatchmakingLog.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

TSharedPtr<FMatchmakingAlgorithmStepStateCreateMatchmakingLobby> FMatchmakingAlgorithmStepCreateMatchmakingLobby::
    CreateNewState()
{
    return MakeShared<FMatchmakingAlgorithmStepStateCreateMatchmakingLobby>();
}

void FMatchmakingAlgorithmStepCreateMatchmakingLobby::OnLobbyCreated(
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

    if (!Task->CurrentStep.IsEqual(MMSTEP_CREATE_MATCHMAKING_LOBBY))
    {
        return;
    }

    if (!Error.bSucceeded)
    {
        if (Error.ErrorCode.EndsWith(".timed_out") || Error.ErrorCode.EndsWith(".already_pending"))
        {
            // Retry on timeout. It's safe to just invoke Start() again for this implementation.
            this->Start(Task.ToSharedRef());
            return;
        }

        this->RestartableError(
            *Task,
            FString::Printf(TEXT("Unable to create lobby for matchmaking: %s"), *Error.ToLogString()));
        return;
    }

    if (auto Pins = Task->PinAndContinue())
    {
        Task->MatchmakingLobbyId = Lobby->Id;

        UE_LOG(
            LogMatchmaking,
            Verbose,
            TEXT("%sCreated a matchmaking lobby: %s"),
            *this->GetLogPrefix(*Task),
            *Task->MatchmakingLobbyId->ToString());

        FMatchmakingTeamId InitialTeamAssignment = Task->TeamManager->GetAssignedTeam(*Task->Request.UserId);
        UE_LOG(
            LogMatchmaking,
            Verbose,
            TEXT("%sHost user %s assigned to team %d"),
            *this->GetLogPrefix(*Task),
            *Task->Request.UserId->ToString(),
            InitialTeamAssignment);

        if (Task->Request.PartyId != nullptr)
        {
            EMIT_TASK_COUNTER_TRACE(Task);
            this->MoveToStep(*Task, MMSTEP_NOTIFY_EXISTING_PARTY);
        }
        else
        {
            EMIT_TASK_COUNTER_TRACE(Task);
            this->MoveToStep(*Task, MMSTEP_WAITING_FOR_PLAYERS_TO_JOIN);
        }
    }
}

void FMatchmakingAlgorithmStepCreateMatchmakingLobby::Start(TSharedRef<FMatchmakingEngineTask> Task)
{
    if (auto Pins = Task->PinAndContinue())
    {
        UE_LOG(LogMatchmaking, Verbose, TEXT("%sCreating a matchmaking lobby"), *this->GetLogPrefix(*Task));

        auto Transaction = Pins->OnlineLobby->MakeCreateLobbyTransaction(*Task->Request.UserId);
        Transaction->Capacity = static_cast<uint32>(Task->TeamManager->GetTotalCapacity());
        Transaction->Locked = false;
        Transaction->Public = true;
        Transaction->SetMetadata.Add(MatchmakingMetadataLobbyType, FVariantData(MatchmakingLobbyTypeMatch));
        Transaction->SetMetadata.Add(
            MatchmakingMetadataQueueName,
            FVariantData(Task->Request.HostConfiguration->QueueName));
        Transaction->SetMetadata.Add(MatchmakingMetadataLobbyStatus, FVariantData(MatchmakingStatusFilling));
        Transaction->SetMetadata.Add(
            MatchmakingMetadataFollowRequestLobbyId,
            FVariantData(Task->MatchmakingFollowRequestLobbyId->ToString()));
        Transaction->SetMetadata.Add(
            MatchmakingMetadataTargetCapacity,
            FVariantData((int64)Task->TeamManager->GetTotalCapacity()));

        // We will not get an OnMemberConnect event for the host, so we need to
        // perform a team assignment update for the host during creation of the lobby.
        FMatchmakingTeamId InitialTeamAssignment = Task->TeamManager->GetAssignedTeam(*Task->Request.UserId);
        UE_LOG(
            LogMatchmaking,
            Verbose,
            TEXT("%sHost user %s is being assigned to team %d..."),
            *this->GetLogPrefix(*Task),
            *Task->Request.UserId->ToString(),
            InitialTeamAssignment);
        Transaction->SetMetadata.Add(
            FString::Printf(TEXT("%s.%s"), *Task->Request.UserId->ToString(), MatchmakingMemberMetadataTeam),
            FVariantData((int64)InitialTeamAssignment));

        if (!Pins->OnlineLobby->CreateLobby(
                *Task->Request.UserId,
                *Transaction,
                FOnLobbyCreateOrConnectComplete::CreateSP(
                    this,
                    &FMatchmakingAlgorithmStepCreateMatchmakingLobby::OnLobbyCreated,
                    TWeakPtr<FMatchmakingEngineTask>(Task))))
        {
            this->RestartableError(*Task, TEXT("Unable to create lobby for matchmaking"));
            return;
        }
    }
}

#endif

REDPOINT_EOS_CODE_GUARD_END()