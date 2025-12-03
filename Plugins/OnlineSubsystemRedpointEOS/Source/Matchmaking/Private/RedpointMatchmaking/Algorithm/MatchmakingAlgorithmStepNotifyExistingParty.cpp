// Copyright June Rhodes. All Rights Reserved.

#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepNotifyExistingParty.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepWaitingForPlayersToJoin.h"
#include "RedpointMatchmaking/MatchmakingConstants.h"
#include "RedpointMatchmaking/MatchmakingLog.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

TSharedPtr<FMatchmakingAlgorithmStepStateNotifyExistingParty> FMatchmakingAlgorithmStepNotifyExistingParty::
    CreateNewState()
{
    return MakeShared<FMatchmakingAlgorithmStepStateNotifyExistingParty>();
}

static FString ExistingParty_GetStringValueOfVariantData(const FVariantData &Data)
{
    FString DataStr;
    if (Data.GetType() == EOnlineKeyValuePairDataType::String)
    {
        Data.GetValue(DataStr);
    }
    return DataStr;
}

void FMatchmakingAlgorithmStepNotifyExistingParty::OnPartyDataUpdated(
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

    if (!Task->CurrentStep.IsEqual(MMSTEP_NOTIFY_EXISTING_PARTY))
    {
        return;
    }

    if (auto Pins = Task->PinAndContinue())
    {
        FVariantData OutLobbyId;
        FString OutLobbyIdStr;
        if (LocalUserId == *Task->Request.UserId && PartyId == *Task->Request.PartyId &&
            PartyData.GetAttribute(PartyMetadataMatchmakingLobbyId, OutLobbyId) &&
            ExistingParty_GetStringValueOfVariantData(OutLobbyId) == *Task->MatchmakingLobbyId->ToString())
        {
            // This is the update event we're looking for, and our party has been updated
            // to the new values.

            TSharedPtr<FMatchmakingAlgorithmStepStateNotifyExistingParty> State = GetState(*Task);
            Pins->OnlineParty->ClearOnPartyDataReceivedDelegate_Handle(State->OnPartyDataReceivedHandle);

            UE_LOG(
                LogMatchmaking,
                Verbose,
                TEXT("%sTold clients what lobby they need to join: %s, now waiting "
                     "for "
                     "them to "
                     "join..."),
                *this->GetLogPrefix(*Task),
                *Task->MatchmakingLobbyId->ToString());

            EMIT_TASK_COUNTER_TRACE(Task);
            this->MoveToStep(*Task, MMSTEP_WAITING_FOR_PLAYERS_TO_JOIN);
        }
    }
}

void FMatchmakingAlgorithmStepNotifyExistingParty::Start(TSharedRef<FMatchmakingEngineTask> Task)
{
    check(Task->Request.PartyId.IsValid());

    if (auto Pins = Task->PinAndContinue())
    {
        UE_LOG(
            LogMatchmaking,
            Verbose,
            TEXT("%sTelling clients what lobby they need to join: %s"),
            *this->GetLogPrefix(*Task),
            *Task->MatchmakingLobbyId->ToString());

        FOnlinePartyDataConstPtr PartyData =
            Pins->OnlineParty->GetPartyData(*Task->Request.UserId, *Task->Request.PartyId, NAME_Default);
        if (PartyData != nullptr)
        {
            TSharedRef<FOnlinePartyData> NewPartyData = MakeShared<FOnlinePartyData>(*PartyData);
            NewPartyData->SetAttribute(
                PartyMetadataMatchmakingLobbyId,
                FVariantData(Task->MatchmakingLobbyId->ToString()));
            NewPartyData->RemoveAttribute(MatchmakingAbandonKey);

            TSharedPtr<FMatchmakingAlgorithmStepStateNotifyExistingParty> State = GetState(*Task);
            State->OnPartyDataReceivedHandle =
                Pins->OnlineParty->AddOnPartyDataReceivedDelegate_Handle(FOnPartyDataReceivedDelegate::CreateSP(
                    this,
                    &FMatchmakingAlgorithmStepNotifyExistingParty::OnPartyDataUpdated,
                    TWeakPtr<FMatchmakingEngineTask>(Task)));

            if (!Pins->OnlineParty
                     ->UpdatePartyData(*Task->Request.UserId, *Task->Request.PartyId, NAME_Default, *NewPartyData))
            {
                Pins->OnlineParty->ClearOnPartyDataReceivedDelegate_Handle(State->OnPartyDataReceivedHandle);
                this->RestartableError(*Task, TEXT("Unable to notify other party members of matchmaking lobby"));
                return;
            }
        }
        else
        {
            this->RestartableError(*Task, TEXT("Unable to retrieve existing party data"));
            return;
        }
    }
}

#endif

REDPOINT_EOS_CODE_GUARD_END()