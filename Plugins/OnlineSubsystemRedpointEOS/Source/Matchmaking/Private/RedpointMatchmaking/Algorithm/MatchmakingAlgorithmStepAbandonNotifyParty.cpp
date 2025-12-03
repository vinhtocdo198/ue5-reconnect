// Copyright June Rhodes. All Rights Reserved.

#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepAbandonNotifyParty.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepAbandonNotifyMatch.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepAbandonWaitForPartyLeader.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepUnlockParty.h"
#include "RedpointMatchmaking/EventHandlers/MatchmakingEventHandlerAbandonment.h"
#include "RedpointMatchmaking/MatchmakingConstants.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

TSharedPtr<FMatchmakingAlgorithmStepStateAbandonNotifyParty> FMatchmakingAlgorithmStepAbandonNotifyParty::
    CreateNewState()
{
    return MakeShared<FMatchmakingAlgorithmStepStateAbandonNotifyParty>();
}

void FMatchmakingAlgorithmStepAbandonNotifyParty::OnPartyDataReceived(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    const FName &Namespace,
    const FOnlinePartyData &PartyData,
    TWeakPtr<FMatchmakingEngineTask> TaskWk,
    EMatchmakingAbandonmentType AbandonmentType,
    FString ExpectedValue)
{
    auto Task = TaskWk.Pin();
    check(Task.IsValid());

    check(Task->CurrentStep.IsEqual(MMSTEP_ABANDON_NOTIFY_PARTY));

    if (LocalUserId != *Task->Request.UserId || PartyId != *Task->Request.PartyId || !Namespace.IsEqual(NAME_Default))
    {
        return;
    }

    FVariantData OutAbandonStatus;
    if (!PartyData.GetAttribute(MatchmakingAbandonKey, OutAbandonStatus) ||
        OutAbandonStatus.GetType() != EOnlineKeyValuePairDataType::String)
    {
        return;
    }
    FString OutAbandonValue;
    OutAbandonStatus.GetValue(OutAbandonValue);
    if (OutAbandonValue != ExpectedValue)
    {
        return;
    }

    if (auto Pins = Task->PinAndContinue())
    {
        TSharedPtr<FMatchmakingAlgorithmStepStateAbandonNotifyParty> State = GetState(*Task);
        Pins->OnlineParty->ClearOnPartyDataReceivedDelegate_Handle(State->OnPartyDataReceivedDelegateHandle);
    }

    this->MoveToStep(*Task, MMSTEP_ABANDON_UNLOCK_PARTY, AbandonmentType);
}

void FMatchmakingAlgorithmStepAbandonNotifyParty::OnPartyMemberDataReceived(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    const FUniqueNetId &MemberId,
    const FName &Namespace,
    const FOnlinePartyData &PartyMemberData,
    TWeakPtr<FMatchmakingEngineTask> TaskWk,
    EMatchmakingAbandonmentType AbandonmentType,
    FString ExpectedValue)
{
    auto Task = TaskWk.Pin();
    check(Task.IsValid());

    check(Task->CurrentStep.IsEqual(MMSTEP_ABANDON_NOTIFY_PARTY));

    if (LocalUserId != *Task->Request.UserId || PartyId != *Task->Request.PartyId ||
        MemberId != *Task->Request.UserId || !Namespace.IsEqual(NAME_Default))
    {
        return;
    }

    FVariantData OutAbandonStatus;
    if (!PartyMemberData.GetAttribute(MatchmakingAbandonKey, OutAbandonStatus) ||
        OutAbandonStatus.GetType() != EOnlineKeyValuePairDataType::String)
    {
        return;
    }
    FString OutAbandonValue;
    OutAbandonStatus.GetValue(OutAbandonValue);
    if (OutAbandonValue != ExpectedValue)
    {
        return;
    }

    if (auto Pins = Task->PinAndContinue())
    {
        TSharedPtr<FMatchmakingAlgorithmStepStateAbandonNotifyParty> State = GetState(*Task);
        Pins->OnlineParty->ClearOnPartyMemberDataReceivedDelegate_Handle(
            State->OnPartyMemberDataReceivedDelegateHandle);
    }

    // We must now wait for the party leader to get the notification and tell us that we're
    // leaving matchmaking. That's because until the party leader acknowledges it and updates
    // the party data, the party data still has the "matchmaking lobby ID to follow". If we restarted
    // client matchmaking before that attribute was removed, we'd re-join a matchmaking lobby incorrectly.
    this->MoveToStep(*Task, MMSTEP_ABANDON_WAIT_FOR_PARTY_LEADER);
}

void FMatchmakingAlgorithmStepAbandonNotifyParty::Start(
    TSharedRef<FMatchmakingEngineTask> Task,
    EMatchmakingAbandonmentType AbandonmentType)
{
    if (auto Pins = Task->PinAndContinue())
    {
        if (Task->Request.PartyId.IsValid() && Pins->OnlineParty.IsValid())
        {
            TSharedPtr<FMatchmakingAlgorithmStepStateAbandonNotifyParty> State = GetState(*Task);

            auto PartyData =
                Pins->OnlineParty->GetPartyData(*Task->Request.UserId, *Task->Request.PartyId, NAME_Default);
            FVariantData ExistingLeaderAbandonmentValue;
            if (PartyData->GetAttribute(MatchmakingAbandonKey, ExistingLeaderAbandonmentValue) &&
                ExistingLeaderAbandonmentValue.GetType() == EOnlineKeyValuePairDataType::String)
            {
                FString ExistingLeaderAbandonmentValueStr;
                ExistingLeaderAbandonmentValue.GetValue(ExistingLeaderAbandonmentValueStr);

                // The party leader has already notified us of abandonment (and this request might come through
                // the abandonment event handler). So we don't need to notify the party leader, we can go
                // straight notifying the match and leaving. Also, honor whatever the party leader wants to do.
                if (ExistingLeaderAbandonmentValueStr.StartsWith(
                        FString::Printf(TEXT("%s-"), MatchmakingAbandonPartyRestart)))
                {
                    this->MoveToStep(*Task, MMSTEP_ABANDON_UNLOCK_PARTY, EMatchmakingAbandonmentType::Restart);
                }
                else
                {
                    this->MoveToStep(*Task, MMSTEP_ABANDON_UNLOCK_PARTY, EMatchmakingAbandonmentType::Cancel);
                }
                return;
            }

            if (Pins->OnlineParty->IsMemberLeader(*Task->Request.UserId, *Task->Request.PartyId, *Task->Request.UserId))
            {
                FString AbandonValue = FString::Printf(
                    TEXT("%s-%lld"),
                    AbandonmentType == EMatchmakingAbandonmentType::Restart ? MatchmakingAbandonPartyRestart
                                                                            : MatchmakingAbandonPartyCancel,
                    FDateTime::UtcNow().ToUnixTimestamp());

                // We notify at the party data level.
                State->OnPartyDataReceivedDelegateHandle =
                    Pins->OnlineParty->AddOnPartyDataReceivedDelegate_Handle(FOnPartyDataReceivedDelegate::CreateSP(
                        this,
                        &FMatchmakingAlgorithmStepAbandonNotifyParty::OnPartyDataReceived,
                        TWeakPtr<FMatchmakingEngineTask>(Task),
                        AbandonmentType,
                        AbandonValue));

                TSharedRef<FOnlinePartyData> Data = MakeShared<FOnlinePartyData>(
                    *Pins->OnlineParty->GetPartyData(*Task->Request.UserId, *Task->Request.PartyId, NAME_Default));
                Data->SetAttribute(MatchmakingAbandonKey, AbandonValue);
                // Party leader also removes PartyMetadataMatchmakingLobbyId attribute, until NotifyExistingParty
                // runs again.
                Data->RemoveAttribute(PartyMetadataMatchmakingLobbyId);
                if (!Pins->OnlineParty
                         ->UpdatePartyData(*Task->Request.UserId, *Task->Request.PartyId, NAME_Default, *Data))
                {
                    UE_LOG(LogMatchmaking, Warning, TEXT("Unable to notify party that we are abandoning matchmaking."));
                    Pins->OnlineParty->ClearOnPartyDataReceivedDelegate_Handle(
                        State->OnPartyDataReceivedDelegateHandle);
                    this->MoveToStep(*Task, MMSTEP_ABANDON_UNLOCK_PARTY, AbandonmentType);
                }
            }
            else
            {
                FString AbandonValue = FString::Printf(
                    TEXT("%s-%lld"),
                    AbandonmentType == EMatchmakingAbandonmentType::Restart ? MatchmakingAbandonPartyRestart
                                                                            : MatchmakingAbandonPartyCancel,
                    FDateTime::UtcNow().ToUnixTimestamp());

                // We notify at the party member level.
                State->OnPartyMemberDataReceivedDelegateHandle =
                    Pins->OnlineParty->AddOnPartyMemberDataReceivedDelegate_Handle(
                        FOnPartyMemberDataReceivedDelegate::CreateSP(
                            this,
                            &FMatchmakingAlgorithmStepAbandonNotifyParty::OnPartyMemberDataReceived,
                            TWeakPtr<FMatchmakingEngineTask>(Task),
                            AbandonmentType,
                            AbandonValue));

                TSharedRef<FOnlinePartyData> Data = MakeShared<FOnlinePartyData>(*Pins->OnlineParty->GetPartyMemberData(
                    *Task->Request.UserId,
                    *Task->Request.PartyId,
                    *Task->Request.UserId,
                    NAME_Default));
                Data->SetAttribute(MatchmakingAbandonKey, AbandonValue);
                if (!Pins->OnlineParty
                         ->UpdatePartyMemberData(*Task->Request.UserId, *Task->Request.PartyId, NAME_Default, *Data))
                {
                    UE_LOG(
                        LogMatchmaking,
                        Warning,
                        TEXT("Unable to notify party leader that we are abandoning matchmaking."));
                    Pins->OnlineParty->ClearOnPartyMemberDataReceivedDelegate_Handle(
                        State->OnPartyMemberDataReceivedDelegateHandle);
                    this->MoveToStep(*Task, MMSTEP_ABANDON_UNLOCK_PARTY, AbandonmentType);
                }
            }

            return;
        }
    }

    this->MoveToStep(*Task, MMSTEP_ABANDON_UNLOCK_PARTY, AbandonmentType);
}

bool FMatchmakingAlgorithmStepAbandonNotifyParty::CanHandleInterrupt(
    const TSharedRef<FMatchmakingEngineTask> &Task,
    FName InterruptType)
{
    if (InterruptType.IsEqual(MMINTERRUPT_HAS_SPECIAL_HANDLING_OF_ABANDONMENT) ||
        InterruptType.IsEqual(MMINTERRUPT_ABANDON_CANCEL) || InterruptType.IsEqual(MMINTERRUPT_ABANDON_RESTART))
    {
        return true;
    }

    return false;
}

bool FMatchmakingAlgorithmStepAbandonNotifyParty::HandleInterrupt(
    const TSharedRef<FMatchmakingEngineTask> &Task,
    FName InterruptType)
{
    if (InterruptType.IsEqual(MMINTERRUPT_HAS_SPECIAL_HANDLING_OF_ABANDONMENT) ||
        InterruptType.IsEqual(MMINTERRUPT_ABANDON_CANCEL) || InterruptType.IsEqual(MMINTERRUPT_ABANDON_RESTART))
    {
        // We're already abandoning.
        return true;
    }

    return false;
}

#endif

REDPOINT_EOS_CODE_GUARD_END()