// Copyright June Rhodes. All Rights Reserved.

#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepAbandonWaitForPartyLeader.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepAbandonNotifyMatch.h"
#include "RedpointMatchmaking/EventHandlers/MatchmakingEventHandlerAbandonment.h"
#include "RedpointMatchmaking/MatchmakingConstants.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

TSharedPtr<FMatchmakingAlgorithmStepStateAbandonWaitForPartyLeader> FMatchmakingAlgorithmStepAbandonWaitForPartyLeader::
    CreateNewState()
{
    return MakeShared<FMatchmakingAlgorithmStepStateAbandonWaitForPartyLeader>();
}

void FMatchmakingAlgorithmStepAbandonWaitForPartyLeader::OnPartyDataReceived(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    const FName &Namespace,
    const FOnlinePartyData &PartyData,
    TWeakPtr<FMatchmakingEngineTask> TaskWk)
{
    auto Task = TaskWk.Pin();
    check(Task.IsValid());

    check(Task->CurrentStep.IsEqual(MMSTEP_ABANDON_WAIT_FOR_PARTY_LEADER));

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
    if (!OutAbandonValue.StartsWith(FString::Printf(TEXT("%s-"), MatchmakingAbandonPartyRestart)) &&
        !OutAbandonValue.StartsWith(FString::Printf(TEXT("%s-"), MatchmakingAbandonPartyCancel)))
    {
        UE_LOG(
            LogMatchmaking,
            Error,
            TEXT("The abandon signal '%s' isn't a recognised value; it will be ignored."),
            *OutAbandonValue);
        return;
    }

    if (auto Pins = Task->PinAndContinue())
    {
        TSharedPtr<FMatchmakingAlgorithmStepStateAbandonWaitForPartyLeader> State = GetState(*Task);
        Pins->OnlineParty->ClearOnPartyDataReceivedDelegate_Handle(State->OnPartyDataReceivedDelegateHandle);
    }

    this->MoveToStep(
        *Task,
        MMSTEP_ABANDON_NOTIFY_MATCH,
        OutAbandonValue.StartsWith(MatchmakingAbandonPartyRestart) ? EMatchmakingAbandonmentType::Restart
                                                                   : EMatchmakingAbandonmentType::Cancel);
}

void FMatchmakingAlgorithmStepAbandonWaitForPartyLeader::Start(TSharedRef<FMatchmakingEngineTask> Task)
{
    if (auto Pins = Task->PinAndContinue())
    {
        if (Task->Request.PartyId.IsValid() && Pins->OnlineParty.IsValid())
        {
            TSharedPtr<FMatchmakingAlgorithmStepStateAbandonWaitForPartyLeader> State = GetState(*Task);

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
                    this->MoveToStep(*Task, MMSTEP_ABANDON_NOTIFY_MATCH, EMatchmakingAbandonmentType::Restart);
                }
                else
                {
                    this->MoveToStep(*Task, MMSTEP_ABANDON_NOTIFY_MATCH, EMatchmakingAbandonmentType::Cancel);
                }
                return;
            }

            // The leader will notify us via party data.
            State->OnPartyDataReceivedDelegateHandle =
                Pins->OnlineParty->AddOnPartyDataReceivedDelegate_Handle(FOnPartyDataReceivedDelegate::CreateSP(
                    this,
                    &FMatchmakingAlgorithmStepAbandonWaitForPartyLeader::OnPartyDataReceived,
                    TWeakPtr<FMatchmakingEngineTask>(Task)));
            return;
        }
    }

    // Uhhh...
    this->MoveToStep(*Task, MMSTEP_ABANDON_NOTIFY_MATCH, EMatchmakingAbandonmentType::Cancel);
}

bool FMatchmakingAlgorithmStepAbandonWaitForPartyLeader::CanHandleInterrupt(
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

bool FMatchmakingAlgorithmStepAbandonWaitForPartyLeader::HandleInterrupt(
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