// Copyright June Rhodes. All Rights Reserved.

#include "MatchmakingEventHandlerAbandonment.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "../Algorithm/MatchmakingAlgorithmStep.h"
#include "../Algorithm/MatchmakingAlgorithmStepAbandonStart.h"
#include "../MatchmakingAbandonmentType.h"
#include "RedpointMatchmaking/MatchmakingConstants.h"
#include "RedpointMatchmaking/MatchmakingLog.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMHANDLER_ABANDONMENT_LOBBY FName("EventHandlerAbandonmentLobby")
#define MMHANDLER_ABANDONMENT_PARTY FName("EventHandlerAbandonmentParty")
#define MMHANDLER_ABANDONMENT_PARTY_MEMBER FName("EventHandlerAbandonmentPartyMember")

void FMatchmakingEventHandlerAbandonment::OnLobbyMemberUpdate(
    const FUniqueNetId &UserId,
    const FOnlineLobbyId &LobbyId,
    const FUniqueNetId &MemberId,
    TSharedRef<FMatchmakingEngineImpl> Engine,
    TWeakPtr<FMatchmakingEngineTask> TaskWk)
{
    auto Task = TaskWk.Pin();
    check(Task.IsValid());

    if (!Task->MatchmakingLobbyId.IsValid())
    {
        return;
    }

    if (UserId != *Task->Request.UserId || LobbyId != *Task->MatchmakingLobbyId)
    {
        return;
    }

    if (auto Pins = Task->PinAndContinue())
    {
        FVariantData OutAbandonmentValue;
        if (Pins->OnlineLobby
                ->GetMemberMetadataValue(UserId, LobbyId, MemberId, MatchmakingAbandonKey, OutAbandonmentValue))
        {
            if (OutAbandonmentValue == MatchmakingAbandonMatch)
            {
                auto Step = Engine->GetRegisteredStep<IMatchmakingAlgorithmStep<IMatchmakingAlgorithmStepState>>(
                    Task->CurrentStep);

                UE_LOG(
                    LogMatchmaking,
                    Verbose,
                    TEXT("%sGot notification from another matchmaking user that they are abandoning matchmaking, going "
                         "into restart."),
                    *this->GetLogPrefix(*Task, MMHANDLER_ABANDONMENT_LOBBY));

                if (Step->CanHandleInterrupt(Task.ToSharedRef(), MMINTERRUPT_HAS_SPECIAL_HANDLING_OF_ABANDONMENT))
                {
                    if (!Step->HandleInterrupt(Task.ToSharedRef(), MMINTERRUPT_ABANDON_RESTART))
                    {
                        Step->MoveToStep(*Task, MMSTEP_ABANDON_START, EMatchmakingAbandonmentType::Restart);
                    }
                }
                else
                {
                    Step->MoveToStep(*Task, MMSTEP_ABANDON_START, EMatchmakingAbandonmentType::Restart);
                }
            }
        }
    }
}

void FMatchmakingEventHandlerAbandonment::OnPartyUpdate(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    const FName &Namespace,
    const FOnlinePartyData &PartyData,
    TSharedRef<FMatchmakingEngineImpl> Engine,
    TWeakPtr<FMatchmakingEngineTask> TaskWk)
{
    auto Task = TaskWk.Pin();
    check(Task.IsValid());

    if (LocalUserId != *Task->Request.UserId || PartyId != *Task->Request.PartyId || !Namespace.IsEqual(NAME_Default))
    {
        return;
    }

    if (auto Pins = Task->PinAndContinue())
    {
        if (Pins->OnlineParty->IsMemberLeader(*Task->Request.UserId, *Task->Request.PartyId, *Task->Request.UserId))
        {
            // Ignore our own update as the leader.
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

        auto Step =
            Engine->GetRegisteredStep<IMatchmakingAlgorithmStep<IMatchmakingAlgorithmStepState>>(Task->CurrentStep);

        FName LastRestartKey(TEXT("owner-restart"));
        if (Task->EventData.Contains(LastRestartKey) && Task->EventData[LastRestartKey] == OutAbandonValue)
        {
            // Already got this notification.
            UE_LOG(
                LogMatchmaking,
                Verbose,
                TEXT("%sIgnoring abandon attribute %s='%s' because we already handled that notification."),
                *this->GetLogPrefix(*Task, MMHANDLER_ABANDONMENT_PARTY_MEMBER),
                *LastRestartKey.ToString(),
                *OutAbandonValue);
            return;
        }
        Task->EventData.Add(LastRestartKey, OutAbandonValue);

        if (OutAbandonValue.StartsWith(FString::Printf(TEXT("%s-"), MatchmakingAbandonPartyRestart)))
        {
            UE_LOG(
                LogMatchmaking,
                Verbose,
                TEXT("%sGot notification from the party leader that matchmaking is being abandoned (restart)."),
                *this->GetLogPrefix(*Task, MMHANDLER_ABANDONMENT_PARTY));

            if (Step->CanHandleInterrupt(Task.ToSharedRef(), MMINTERRUPT_HAS_SPECIAL_HANDLING_OF_ABANDONMENT))
            {
                if (!Step->HandleInterrupt(Task.ToSharedRef(), MMINTERRUPT_ABANDON_RESTART))
                {
                    Step->MoveToStep(*Task, MMSTEP_ABANDON_START, EMatchmakingAbandonmentType::Restart);
                }
            }
            else
            {
                Step->MoveToStep(*Task, MMSTEP_ABANDON_START, EMatchmakingAbandonmentType::Restart);
            }
        }
        else if (OutAbandonValue.StartsWith(FString::Printf(TEXT("%s-"), MatchmakingAbandonPartyCancel)))
        {
            UE_LOG(
                LogMatchmaking,
                Verbose,
                TEXT("%sGot notification from the party leader that matchmaking is being abandoned (cancel)."),
                *this->GetLogPrefix(*Task, MMHANDLER_ABANDONMENT_PARTY));

            if (Step->CanHandleInterrupt(Task.ToSharedRef(), MMINTERRUPT_HAS_SPECIAL_HANDLING_OF_ABANDONMENT))
            {
                if (!Step->HandleInterrupt(Task.ToSharedRef(), MMINTERRUPT_ABANDON_CANCEL))
                {
                    Step->MoveToStep(*Task, MMSTEP_ABANDON_START, EMatchmakingAbandonmentType::Cancel);
                }
            }
            else
            {
                Step->MoveToStep(*Task, MMSTEP_ABANDON_START, EMatchmakingAbandonmentType::Cancel);
            }
        }
        else
        {
            checkf(false, TEXT("Invalid abandonment value."));
        }
    }
}

void FMatchmakingEventHandlerAbandonment::OnPartyMemberUpdate(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    const FUniqueNetId &MemberId,
    const FName &Namespace,
    const FOnlinePartyData &PartyMemberData,
    TSharedRef<FMatchmakingEngineImpl> Engine,
    TWeakPtr<FMatchmakingEngineTask> TaskWk)
{
    auto Task = TaskWk.Pin();
    check(Task.IsValid());

    if (LocalUserId != *Task->Request.UserId || PartyId != *Task->Request.PartyId ||
        MemberId == *Task->Request.UserId || !Namespace.IsEqual(NAME_Default))
    {
        return;
    }

    if (auto Pins = Task->PinAndContinue())
    {
        if (!Pins->OnlineParty->IsMemberLeader(*Task->Request.UserId, *Task->Request.PartyId, *Task->Request.UserId))
        {
            // This is a notification from a non-leader in the party. We'll wait for the leader to receive this
            // notification and then propagate abandonment back down to us through a party data update.
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

        auto Step =
            Engine->GetRegisteredStep<IMatchmakingAlgorithmStep<IMatchmakingAlgorithmStepState>>(Task->CurrentStep);

        FName LastRestartKey(*FString::Printf(TEXT("%s-restart"), *MemberId.ToString()));
        if (Task->EventData.Contains(LastRestartKey) && Task->EventData[LastRestartKey] == OutAbandonValue)
        {
            // Already got this notification.
            UE_LOG(
                LogMatchmaking,
                Verbose,
                TEXT("%sIgnoring abandon attribute %s='%s' because we already handled that notification."),
                *this->GetLogPrefix(*Task, MMHANDLER_ABANDONMENT_PARTY_MEMBER),
                *LastRestartKey.ToString(),
                *OutAbandonValue);
            return;
        }
        Task->EventData.Add(LastRestartKey, OutAbandonValue);

        if (OutAbandonValue.StartsWith(FString::Printf(TEXT("%s-"), MatchmakingAbandonPartyRestart)))
        {
            UE_LOG(
                LogMatchmaking,
                Verbose,
                TEXT("%sGot notification from another party member that matchmaking is being abandoned (restart)."),
                *this->GetLogPrefix(*Task, MMHANDLER_ABANDONMENT_PARTY_MEMBER));

            if (Step->CanHandleInterrupt(Task.ToSharedRef(), MMINTERRUPT_HAS_SPECIAL_HANDLING_OF_ABANDONMENT))
            {
                if (!Step->HandleInterrupt(Task.ToSharedRef(), MMINTERRUPT_ABANDON_RESTART))
                {
                    Step->MoveToStep(*Task, MMSTEP_ABANDON_START, EMatchmakingAbandonmentType::Restart);
                }
            }
            else
            {
                Step->MoveToStep(*Task, MMSTEP_ABANDON_START, EMatchmakingAbandonmentType::Restart);
            }
        }
        else if (OutAbandonValue.StartsWith(FString::Printf(TEXT("%s-"), MatchmakingAbandonPartyCancel)))
        {
            UE_LOG(
                LogMatchmaking,
                Verbose,
                TEXT("%sGot notification from another party member that matchmaking is being abandoned (cancel)."),
                *this->GetLogPrefix(*Task, MMHANDLER_ABANDONMENT_PARTY_MEMBER));

            if (Step->CanHandleInterrupt(Task.ToSharedRef(), MMINTERRUPT_HAS_SPECIAL_HANDLING_OF_ABANDONMENT))
            {
                if (!Step->HandleInterrupt(Task.ToSharedRef(), MMINTERRUPT_ABANDON_CANCEL))
                {
                    Step->MoveToStep(*Task, MMSTEP_ABANDON_START, EMatchmakingAbandonmentType::Cancel);
                }
            }
            else
            {
                Step->MoveToStep(*Task, MMSTEP_ABANDON_START, EMatchmakingAbandonmentType::Cancel);
            }
        }
        else
        {
            checkf(false, TEXT("Invalid abandonment value."));
        }
    }
}

void FMatchmakingEventHandlerAbandonment::RegisterEvents(FMatchmakingEngineImpl &Engine, FMatchmakingEngineTask &Task)
{
    if (auto Pins = Task.PinAndContinue())
    {
        Task.EventHandlers.Add(
            MMHANDLER_ABANDONMENT_LOBBY,
            Pins->OnlineLobby->AddOnMemberUpdateDelegate_Handle(FOnMemberUpdateDelegate::CreateSP(
                this,
                &FMatchmakingEventHandlerAbandonment::OnLobbyMemberUpdate,
                Engine.AsShared(),
                TWeakPtr<FMatchmakingEngineTask>(Task.AsShared()))));

        if (Pins->OnlineParty.IsValid())
        {
            Task.EventHandlers.Add(
                MMHANDLER_ABANDONMENT_PARTY,
                Pins->OnlineParty->AddOnPartyDataReceivedDelegate_Handle(FOnPartyDataReceivedDelegate::CreateSP(
                    this,
                    &FMatchmakingEventHandlerAbandonment::OnPartyUpdate,
                    Engine.AsShared(),
                    TWeakPtr<FMatchmakingEngineTask>(Task.AsShared()))));
            Task.EventHandlers.Add(
                MMHANDLER_ABANDONMENT_PARTY_MEMBER,
                Pins->OnlineParty->AddOnPartyMemberDataReceivedDelegate_Handle(
                    FOnPartyMemberDataReceivedDelegate::CreateSP(
                        this,
                        &FMatchmakingEventHandlerAbandonment::OnPartyMemberUpdate,
                        Engine.AsShared(),
                        TWeakPtr<FMatchmakingEngineTask>(Task.AsShared()))));
        }

        EMIT_TASK_COUNTER_TRACE(Task.AsShared());
    }
}

void FMatchmakingEventHandlerAbandonment::UnregisterEvents(FMatchmakingEngineImpl &Engine, FMatchmakingEngineTask &Task)
{
    if (auto Pins = Task.PinAndContinue())
    {
        if (Task.EventHandlers.Contains(MMHANDLER_ABANDONMENT_LOBBY))
        {
            Pins->OnlineLobby->ClearOnMemberUpdateDelegate_Handle(Task.EventHandlers[MMHANDLER_ABANDONMENT_LOBBY]);
            Task.EventHandlers.Remove(MMHANDLER_ABANDONMENT_LOBBY);
        }

        if (Pins->OnlineParty.IsValid())
        {
            if (Task.EventHandlers.Contains(MMHANDLER_ABANDONMENT_PARTY))
            {
                Pins->OnlineParty->ClearOnPartyDataReceivedDelegate_Handle(
                    Task.EventHandlers[MMHANDLER_ABANDONMENT_PARTY]);
                Task.EventHandlers.Remove(MMHANDLER_ABANDONMENT_PARTY);
            }

            if (Task.EventHandlers.Contains(MMHANDLER_ABANDONMENT_PARTY_MEMBER))
            {
                Pins->OnlineParty->ClearOnPartyMemberDataReceivedDelegate_Handle(
                    Task.EventHandlers[MMHANDLER_ABANDONMENT_PARTY_MEMBER]);
                Task.EventHandlers.Remove(MMHANDLER_ABANDONMENT_PARTY_MEMBER);
            }
        }

        EMIT_TASK_COUNTER_TRACE(Task.AsShared());
    }
}

#endif

REDPOINT_EOS_CODE_GUARD_END()