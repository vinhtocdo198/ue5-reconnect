// Copyright June Rhodes. All Rights Reserved.

#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepWaitingForPlayersToJoin.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepAbandonStart.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepSearching.h"
#include "RedpointMatchmaking/EventHandlers/MatchmakingEventHandlerTeamManagement.h"
#include "RedpointMatchmaking/MatchmakingConstants.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

TSharedPtr<FMatchmakingAlgorithmStepStateWaitingForPlayersToJoin> FMatchmakingAlgorithmStepWaitingForPlayersToJoin::
    CreateNewState()
{
    return MakeShared<FMatchmakingAlgorithmStepStateWaitingForPlayersToJoin>();
}

void FMatchmakingAlgorithmStepWaitingForPlayersToJoin::Start(TSharedRef<FMatchmakingEngineTask> Task)
{
    if (Task->TeamManager->HasPendingBatch())
    {
        if (Task->TeamManager->IsPendingBatchReady())
        {
            // Our pending users are already ready, jump immediately to search.
            Task->TeamManager->FinalizePendingBatch();
            EMIT_TASK_COUNTER_TRACE(Task);
            this->MoveToStep(*Task, MMSTEP_SEARCHING, true);
            return;
        }
    }
    else
    {
        // We don't have a pending batch, jump immediately to search.
        EMIT_TASK_COUNTER_TRACE(Task);
        this->MoveToStep(*Task, MMSTEP_SEARCHING, true);
        return;
    }

    auto State = this->GetState(*Task);
    State->OnTimeoutHandle = Task->Schedule.Execute(
        MatchmakingTimingWaitForPlayersTimeout,
        FMatchmakingEngineTaskScheduleCallback::CreateSP(
            this,
            &FMatchmakingAlgorithmStepWaitingForPlayersToJoin::OnTimeout,
            TWeakPtr<FMatchmakingEngineTask>(Task)));
}

void FMatchmakingAlgorithmStepWaitingForPlayersToJoin::OnTimeout(TWeakPtr<FMatchmakingEngineTask> TaskWk)
{
    // We ran out of time waiting for players. Restart matchmaking.
    if (auto Task = TaskWk.Pin())
    {
        if (Task.IsValid() && Task->CurrentStep.IsEqual(MMSTEP_WAITING_FOR_PLAYERS_TO_JOIN))
        {
            this->MoveToStep(*Task, MMSTEP_ABANDON_START, EMatchmakingAbandonmentType::Restart);
        }
    }
}

bool FMatchmakingAlgorithmStepWaitingForPlayersToJoin::CanHandleInterrupt(
    const TSharedRef<FMatchmakingEngineTask> &Task,
    FName InterruptType)
{
    return InterruptType.IsEqual(MMINTERRUPT_ALL_PLAYERS_HAVE_CONNECTED);
}

static FString PlayersToJoin_GetStringValueOfVariantData(const FVariantData &Data)
{
    FString DataStr;
    if (Data.GetType() == EOnlineKeyValuePairDataType::String)
    {
        Data.GetValue(DataStr);
    }
    return DataStr;
}

bool FMatchmakingAlgorithmStepWaitingForPlayersToJoin::HandleInterrupt(
    const TSharedRef<FMatchmakingEngineTask> &Task,
    FName InterruptType)
{
    if (InterruptType.IsEqual(MMINTERRUPT_ALL_PLAYERS_HAVE_CONNECTED))
    {
        auto State = this->GetState(*Task);
        Task->Unschedule.Execute(State->OnTimeoutHandle);

        // If the request has a party set, and the party still has the "matchmaking lobby ID" set, clear it
        // now as it's no longer required (all party members are in the match lobby).
        if (auto Pins = Task->PinAndContinue())
        {
            if (Task->Request.PartyId != nullptr)
            {
                FVariantData OutLobbyId;
                FString OutLobbyIdStr;
                FOnlinePartyDataConstPtr PartyData =
                    Pins->OnlineParty->GetPartyData(*Task->Request.UserId, *Task->Request.PartyId, NAME_None);
                if (PartyData != nullptr && PartyData->GetAttribute(PartyMetadataMatchmakingLobbyId, OutLobbyId) &&
                    PlayersToJoin_GetStringValueOfVariantData(OutLobbyId) == *Task->MatchmakingLobbyId->ToString())
                {
                    UE_LOG(
                        LogMatchmaking,
                        Verbose,
                        TEXT("%sClearing %s attribute from party, as it is no longer required..."),
                        *this->GetLogPrefix(*Task),
                        PartyMetadataMatchmakingLobbyId);

                    TSharedRef<FOnlinePartyData> NewPartyData = MakeShared<FOnlinePartyData>(*PartyData);
                    NewPartyData->RemoveAttribute(PartyMetadataMatchmakingLobbyId);

                    State->OnPartyDataReceivedHandle =
                        Pins->OnlineParty->AddOnPartyDataReceivedDelegate_Handle(FOnPartyDataReceivedDelegate::CreateSP(
                            this,
                            &FMatchmakingAlgorithmStepWaitingForPlayersToJoin::OnPartyDataUpdated,
                            TWeakPtr<FMatchmakingEngineTask>(Task)));

                    if (!Pins->OnlineParty->UpdatePartyData(
                            *Task->Request.UserId,
                            *Task->Request.PartyId,
                            NAME_Default,
                            *NewPartyData))
                    {
                        Pins->OnlineParty->ClearOnPartyDataReceivedDelegate_Handle(State->OnPartyDataReceivedHandle);
                        this->RestartableError(
                            *Task,
                            TEXT("Unable to clear matchmaking lobby ID attribute from party data."));
                        return true;
                    }

                    // Don't move to the searching step now, wait until the party data is updated first.
                    return true;
                }
            }
        }

        EMIT_TASK_COUNTER_TRACE(Task);
        this->MoveToStep(*Task, MMSTEP_SEARCHING, true);
        return true;
    }

    return false;
}

void FMatchmakingAlgorithmStepWaitingForPlayersToJoin::OnPartyDataUpdated(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    const FName &Namespace,
    const FOnlinePartyData &PartyData,
    TWeakPtr<FMatchmakingEngineTask> TaskWk)
{
    auto Task = TaskWk.Pin();
    if (!Task.IsValid())
    {
        return;
    }

    if (!Task->CurrentStep.IsEqual(MMSTEP_WAITING_FOR_PLAYERS_TO_JOIN))
    {
        return;
    }

    if (auto Pins = Task->PinAndContinue())
    {
        FVariantData OutLobbyId;
        FString OutLobbyIdStr;
        if (LocalUserId == *Task->Request.UserId && PartyId == *Task->Request.PartyId &&
            !PartyData.GetAttribute(PartyMetadataMatchmakingLobbyId, OutLobbyId))
        {
            // This is the update event we're looking for, and our matchmaking lobby ID has been
            // removed from the party data.

            TSharedPtr<FMatchmakingAlgorithmStepStateWaitingForPlayersToJoin> State = GetState(*Task);
            Pins->OnlineParty->ClearOnPartyDataReceivedDelegate_Handle(State->OnPartyDataReceivedHandle);

            UE_LOG(
                LogMatchmaking,
                Verbose,
                TEXT("%sCleared %s attribute from party, now moving to searching step."),
                *this->GetLogPrefix(*Task),
                PartyMetadataMatchmakingLobbyId);

            EMIT_TASK_COUNTER_TRACE(Task);
            this->MoveToStep(*Task, MMSTEP_SEARCHING, true);
        }
    }
}

#endif

REDPOINT_EOS_CODE_GUARD_END()