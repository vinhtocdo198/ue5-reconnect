// Copyright June Rhodes. All Rights Reserved.

#include "MatchmakingEventHandlerPartyChange.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "../Algorithm/MatchmakingAlgorithmStep.h"
#include "../Algorithm/MatchmakingAlgorithmStepAbandonStart.h"
#include "MatchmakingEventHandlerAbandonment.h"
#include "RedpointMatchmaking/MatchmakingConstants.h"
#include "RedpointMatchmaking/MatchmakingLog.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMHANDLER_PARTY_CHANGE_EXITED FName("EventHandlerPartyChangeExited")
#define MMHANDLER_PARTY_CHANGE_MEMBER_JOINED FName("EventHandlerPartyChangeMemberJoined")
#define MMHANDLER_PARTY_CHANGE_MEMBER_EXITED FName("EventHandlerPartyChangeMemberExited")
#define MMHANDLER_PARTY_CHANGE_MEMBER_PROMOTED FName("EventHandlerPartyChangeMemberPromoted")

void FMatchmakingEventHandlerPartyChange::OnPartyExited(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    TSharedRef<FMatchmakingEngineImpl> Engine,
    TWeakPtr<FMatchmakingEngineTask> TaskWk)
{
    auto Task = TaskWk.Pin();
    check(Task.IsValid());

    if (!Task->Request.PartyId.IsValid() || LocalUserId != *Task->Request.UserId || PartyId != *Task->Request.PartyId)
    {
        return;
    }

    UE_LOG(
        LogMatchmaking,
        Verbose,
        TEXT("%sLeft the party in the middle of matchmaking, cancelling matchmaking..."),
        *this->GetLogPrefix(*Task, MMHANDLER_PARTY_CHANGE_EXITED));
    auto Step = Engine->GetRegisteredStep<IMatchmakingAlgorithmStep<IMatchmakingAlgorithmStepState>>(Task->CurrentStep);
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

void FMatchmakingEventHandlerPartyChange::OnPartyMemberJoined(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    const FUniqueNetId &MemberId,
    TSharedRef<FMatchmakingEngineImpl> Engine,
    TWeakPtr<FMatchmakingEngineTask> TaskWk)
{
    auto Task = TaskWk.Pin();
    check(Task.IsValid());

    if (!Task->Request.PartyId.IsValid() || LocalUserId != *Task->Request.UserId || PartyId != *Task->Request.PartyId)
    {
        return;
    }
    if (!Task->Request.HostConfiguration.IsValid())
    {
        // Clients don't have authority over this; they might be sitting in a partially filled party in the
        // WaitForPartyLeader step, and the party leader intends to enter matchmaking once all the clients are in the
        // party. So leave it up to the party leader to manage membership checks.
        return;
    }

    UE_LOG(
        LogMatchmaking,
        Verbose,
        TEXT("%sSomeone else joined the party in the middle of matchmaking, cancelling matchmaking..."),
        *this->GetLogPrefix(*Task, MMHANDLER_PARTY_CHANGE_EXITED));
    auto Step = Engine->GetRegisteredStep<IMatchmakingAlgorithmStep<IMatchmakingAlgorithmStepState>>(Task->CurrentStep);
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

void FMatchmakingEventHandlerPartyChange::OnPartyMemberExited(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    const FUniqueNetId &MemberId,
    const EMemberExitedReason Reason,
    TSharedRef<FMatchmakingEngineImpl> Engine,
    TWeakPtr<FMatchmakingEngineTask> TaskWk)
{
    auto Task = TaskWk.Pin();
    check(Task.IsValid());

    if (!Task->Request.PartyId.IsValid() || LocalUserId != *Task->Request.UserId || PartyId != *Task->Request.PartyId)
    {
        return;
    }
    if (!Task->Request.HostConfiguration.IsValid())
    {
        // Clients don't have authority over this; they might be sitting in a partially filled party in the
        // WaitForPartyLeader step, and the party leader intends to enter matchmaking once all the clients are in the
        // party. So leave it up to the party leader to manage membership checks.
        return;
    }

    UE_LOG(
        LogMatchmaking,
        Verbose,
        TEXT("%sSomeone else left the party in the middle of matchmaking, cancelling matchmaking..."),
        *this->GetLogPrefix(*Task, MMHANDLER_PARTY_CHANGE_EXITED));
    auto Step = Engine->GetRegisteredStep<IMatchmakingAlgorithmStep<IMatchmakingAlgorithmStepState>>(Task->CurrentStep);
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

void FMatchmakingEventHandlerPartyChange::OnPartyMemberPromoted(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    const FUniqueNetId &NewLeaderId,
    TSharedRef<FMatchmakingEngineImpl> Engine,
    TWeakPtr<FMatchmakingEngineTask> TaskWk)
{
    auto Task = TaskWk.Pin();
    check(Task.IsValid());

    if (!Task->Request.PartyId.IsValid() || LocalUserId != *Task->Request.UserId || PartyId != *Task->Request.PartyId)
    {
        return;
    }
    if (!Task->Request.HostConfiguration.IsValid() && NewLeaderId != LocalUserId)
    {
        // Only the party leader cares about this; clients can remain in WaitForPartyLeader step as long as they don't
        // become the leader.
        return;
    }

    UE_LOG(
        LogMatchmaking,
        Verbose,
        TEXT("%sThe party leader changed in the middle of matchmaking, cancelling matchmaking..."),
        *this->GetLogPrefix(*Task, MMHANDLER_PARTY_CHANGE_EXITED));
    auto Step = Engine->GetRegisteredStep<IMatchmakingAlgorithmStep<IMatchmakingAlgorithmStepState>>(Task->CurrentStep);
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

void FMatchmakingEventHandlerPartyChange::RegisterEvents(FMatchmakingEngineImpl &Engine, FMatchmakingEngineTask &Task)
{
    if (auto Pins = Task.PinAndContinue())
    {
        if (Pins->OnlineParty.IsValid())
        {
            Task.EventHandlers.Add(
                MMHANDLER_PARTY_CHANGE_EXITED,
                Pins->OnlineParty->AddOnPartyExitedDelegate_Handle(FOnPartyExitedDelegate::CreateSP(
                    this,
                    &FMatchmakingEventHandlerPartyChange::OnPartyExited,
                    Engine.AsShared(),
                    TWeakPtr<FMatchmakingEngineTask>(Task.AsShared()))));
            Task.EventHandlers.Add(
                MMHANDLER_PARTY_CHANGE_MEMBER_JOINED,
                Pins->OnlineParty->AddOnPartyMemberJoinedDelegate_Handle(FOnPartyMemberJoinedDelegate::CreateSP(
                    this,
                    &FMatchmakingEventHandlerPartyChange::OnPartyMemberJoined,
                    Engine.AsShared(),
                    TWeakPtr<FMatchmakingEngineTask>(Task.AsShared()))));
            Task.EventHandlers.Add(
                MMHANDLER_PARTY_CHANGE_MEMBER_EXITED,
                Pins->OnlineParty->AddOnPartyMemberExitedDelegate_Handle(FOnPartyMemberExitedDelegate::CreateSP(
                    this,
                    &FMatchmakingEventHandlerPartyChange::OnPartyMemberExited,
                    Engine.AsShared(),
                    TWeakPtr<FMatchmakingEngineTask>(Task.AsShared()))));
            Task.EventHandlers.Add(
                MMHANDLER_PARTY_CHANGE_MEMBER_PROMOTED,
                Pins->OnlineParty->AddOnPartyMemberPromotedDelegate_Handle(FOnPartyMemberPromotedDelegate::CreateSP(
                    this,
                    &FMatchmakingEventHandlerPartyChange::OnPartyMemberPromoted,
                    Engine.AsShared(),
                    TWeakPtr<FMatchmakingEngineTask>(Task.AsShared()))));
        }

        EMIT_TASK_COUNTER_TRACE(Task.AsShared());
    }
}

void FMatchmakingEventHandlerPartyChange::UnregisterEvents(FMatchmakingEngineImpl &Engine, FMatchmakingEngineTask &Task)
{
    if (auto Pins = Task.PinAndContinue())
    {
        if (Pins->OnlineParty.IsValid())
        {
            if (Task.EventHandlers.Contains(MMHANDLER_PARTY_CHANGE_EXITED))
            {
                Pins->OnlineParty->ClearOnPartyExitedDelegate_Handle(Task.EventHandlers[MMHANDLER_PARTY_CHANGE_EXITED]);
                Task.EventHandlers.Remove(MMHANDLER_PARTY_CHANGE_EXITED);
            }
            if (Task.EventHandlers.Contains(MMHANDLER_PARTY_CHANGE_MEMBER_JOINED))
            {
                Pins->OnlineParty->ClearOnPartyMemberJoinedDelegate_Handle(
                    Task.EventHandlers[MMHANDLER_PARTY_CHANGE_MEMBER_JOINED]);
                Task.EventHandlers.Remove(MMHANDLER_PARTY_CHANGE_MEMBER_JOINED);
            }
            if (Task.EventHandlers.Contains(MMHANDLER_PARTY_CHANGE_MEMBER_EXITED))
            {
                Pins->OnlineParty->ClearOnPartyMemberExitedDelegate_Handle(
                    Task.EventHandlers[MMHANDLER_PARTY_CHANGE_MEMBER_EXITED]);
                Task.EventHandlers.Remove(MMHANDLER_PARTY_CHANGE_MEMBER_EXITED);
            }
            if (Task.EventHandlers.Contains(MMHANDLER_PARTY_CHANGE_MEMBER_PROMOTED))
            {
                Pins->OnlineParty->ClearOnPartyMemberPromotedDelegate_Handle(
                    Task.EventHandlers[MMHANDLER_PARTY_CHANGE_MEMBER_PROMOTED]);
                Task.EventHandlers.Remove(MMHANDLER_PARTY_CHANGE_MEMBER_PROMOTED);
            }
        }

        EMIT_TASK_COUNTER_TRACE(Task.AsShared());
    }
}

#endif

REDPOINT_EOS_CODE_GUARD_END()