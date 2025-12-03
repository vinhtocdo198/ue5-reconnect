// Copyright June Rhodes. All Rights Reserved.

#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepUnlockParty.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepAbandonNotifyMatch.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepCompleteMatchAsHost.h"
#include "RedpointMatchmaking/MatchmakingErrors.h"
#include "RedpointMatchmaking/MatchmakingLog.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

void FMatchmakingAlgorithmStepUnlockParty::UnlockPartyIfNeeded(
    TSharedRef<FMatchmakingEngineTask> Task,
    FOnInternalPartyUnlocked OnComplete)
{
    if (auto Pins = Task->PinAndContinue())
    {
        if (Task->Request.PartyId != nullptr)
        {
            if (Task->PartyMustBeUnlocked)
            {
                FOnlinePartyConstPtr PartyInfo =
                    Pins->OnlineParty->GetParty(*Task->Request.UserId, *Task->Request.PartyId);
                if (PartyInfo != nullptr)
                {
                    TSharedRef<FPartyConfiguration> Config =
                        MakeShared<FPartyConfiguration>(*PartyInfo->GetConfiguration());
                    Config->bIsAcceptingMembers = true;
                    if (!Pins->OnlineParty->UpdateParty(
                            *Task->Request.UserId,
                            *Task->Request.PartyId,
                            *Config,
                            false,
                            FOnUpdatePartyComplete::CreateSP(
                                this->AsSharedBase(),
                                &FMatchmakingAlgorithmStepUnlockParty::OnPartyUnlocked,
                                TWeakPtr<FMatchmakingEngineTask>(Task),
                                OnComplete)))
                    {
                        UE_LOG(LogMatchmaking, Warning, TEXT("Unable to unlock party after matchmaking"));
                        OnComplete.ExecuteIfBound();
                        return;
                    }
                }
            }
            else
            {
                OnComplete.ExecuteIfBound();
            }
        }
        else
        {
            OnComplete.ExecuteIfBound();
        }
    }
}

void FMatchmakingAlgorithmStepUnlockParty::OnPartyUnlocked(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    const EUpdateConfigCompletionResult Result,
    TWeakPtr<FMatchmakingEngineTask> TaskWk,
    FOnInternalPartyUnlocked OnComplete)
{
    auto Task = TaskWk.Pin();
    if (!Task.IsValid())
    {
        return;
    }

    if (!Task->CurrentStep.IsEqual(MMSTEP_ABANDON_UNLOCK_PARTY) &&
        !Task->CurrentStep.IsEqual(MMSTEP_COMPLETE_UNLOCK_PARTY))
    {
        return;
    }

    if (auto Pins = Task->PinAndContinue())
    {
        switch (Result)
        {
        case EUpdateConfigCompletionResult::UnknownClientFailure:
            UE_LOG(LogMatchmaking, Warning, TEXT("Unable to unlock party after matchmaking: UnknownClientFailure"));
            break;
        case EUpdateConfigCompletionResult::UnknownParty:
            UE_LOG(LogMatchmaking, Warning, TEXT("Unable to unlock party after matchmaking: UnknownParty"));
            break;
        case EUpdateConfigCompletionResult::LocalMemberNotMember:
            UE_LOG(LogMatchmaking, Warning, TEXT("Unable to unlock party after matchmaking: LocalMemberNotMember"));
            break;
        case EUpdateConfigCompletionResult::LocalMemberNotLeader:
            UE_LOG(LogMatchmaking, Warning, TEXT("Unable to unlock party after matchmaking: LocalMemberNotLeader"));
            break;
        case EUpdateConfigCompletionResult::RemoteMemberNotMember:
            UE_LOG(LogMatchmaking, Warning, TEXT("Unable to unlock party after matchmaking: RemoteMemberNotMember"));
            break;
        case EUpdateConfigCompletionResult::MessagingFailure:
            UE_LOG(LogMatchmaking, Warning, TEXT("Unable to unlock party after matchmaking: MessagingFailure"));
            break;
        case EUpdateConfigCompletionResult::NoResponse:
            UE_LOG(LogMatchmaking, Warning, TEXT("Unable to unlock party after matchmaking: NoResponse"));
            break;
        case EUpdateConfigCompletionResult::UnknownInternalFailure:
            UE_LOG(LogMatchmaking, Warning, TEXT("Unable to unlock party after matchmaking: UnknownInternalFailure"));
            break;
        case EUpdateConfigCompletionResult::Succeeded:
            break;
        default:
            UE_LOG(LogMatchmaking, Warning, TEXT("Unable to unlock party after matchmaking"));
            break;
        }

        EMIT_TASK_COUNTER_TRACE(Task);
        OnComplete.ExecuteIfBound();
    }
}

TSharedPtr<FMatchmakingAlgorithmStepStateUnlockParty> FMatchmakingAlgorithmStepAbandonUnlockParty::CreateNewState()
{
    return MakeShared<FMatchmakingAlgorithmStepStateUnlockParty>();
}

void FMatchmakingAlgorithmStepAbandonUnlockParty::OnComplete(
    TWeakPtr<FMatchmakingEngineTask> TaskWk,
    EMatchmakingAbandonmentType AbandonmentType)
{
    auto Task = TaskWk.Pin();
    if (!Task.IsValid())
    {
        return;
    }

    if (!Task->CurrentStep.IsEqual(MMSTEP_ABANDON_UNLOCK_PARTY))
    {
        return;
    }

    if (Task->Request.PartyId.IsValid())
    {
        UE_LOG(
            LogMatchmaking,
            Verbose,
            TEXT("%sFinished unlocking the party (%s)"),
            *this->GetLogPrefix(*Task),
            *Task->Request.PartyId->ToString());
    }

    EMIT_TASK_COUNTER_TRACE(Task);
    this->MoveToStep(*Task, MMSTEP_ABANDON_NOTIFY_MATCH, AbandonmentType);
}

TSharedRef<FMatchmakingAlgorithmStepUnlockParty> FMatchmakingAlgorithmStepAbandonUnlockParty::AsSharedBase()
{
    return StaticCastSharedRef<FMatchmakingAlgorithmStepUnlockParty>(this->AsShared());
}

void FMatchmakingAlgorithmStepAbandonUnlockParty::Start(
    TSharedRef<FMatchmakingEngineTask> Task,
    EMatchmakingAbandonmentType AbandonmentType)
{
    if (AbandonmentType == EMatchmakingAbandonmentType::Cancel)
    {
        if (Task->Request.PartyId.IsValid())
        {
            UE_LOG(
                LogMatchmaking,
                Verbose,
                TEXT("%sUnlocking the party (%s) so that people can join again"),
                *this->GetLogPrefix(*Task),
                *Task->Request.PartyId->ToString());
        }

        this->UnlockPartyIfNeeded(
            Task,
            FMatchmakingAlgorithmStepUnlockParty::FOnInternalPartyUnlocked::CreateSP(
                this,
                &FMatchmakingAlgorithmStepAbandonUnlockParty::OnComplete,
                TWeakPtr<FMatchmakingEngineTask>(Task),
                AbandonmentType));
    }
    else
    {
        EMIT_TASK_COUNTER_TRACE(Task);
        this->MoveToStep(*Task, MMSTEP_ABANDON_NOTIFY_MATCH, AbandonmentType);
    }
}

TSharedPtr<FMatchmakingAlgorithmStepStateUnlockParty> FMatchmakingAlgorithmStepCompleteUnlockParty::CreateNewState()
{
    return MakeShared<FMatchmakingAlgorithmStepStateUnlockParty>();
}

void FMatchmakingAlgorithmStepCompleteUnlockParty::OnComplete(
    TWeakPtr<FMatchmakingEngineTask> TaskWk)
{
    auto Task = TaskWk.Pin();
    if (!Task.IsValid())
    {
        return;
    }

    if (!Task->CurrentStep.IsEqual(MMSTEP_COMPLETE_UNLOCK_PARTY))
    {
        return;
    }

    if (Task->Request.PartyId.IsValid())
    {
        UE_LOG(
            LogMatchmaking,
            Verbose,
            TEXT("%sFinished unlocking the party (%s)"),
            *this->GetLogPrefix(*Task),
            *Task->Request.PartyId->ToString());
    }

    EMIT_TASK_COUNTER_TRACE(Task);
    this->MoveToStep(*Task, MMSTEP_COMPLETE_MATCH_AS_HOST);
}

TSharedRef<FMatchmakingAlgorithmStepUnlockParty> FMatchmakingAlgorithmStepCompleteUnlockParty::AsSharedBase()
{
    return StaticCastSharedRef<FMatchmakingAlgorithmStepUnlockParty>(this->AsShared());
}

void FMatchmakingAlgorithmStepCompleteUnlockParty::Start(TSharedRef<FMatchmakingEngineTask> Task)
{
    if (Task->Request.PartyId.IsValid())
    {
        UE_LOG(
            LogMatchmaking,
            Verbose,
            TEXT("%sUnlocking the party (%s) so that people can join again"),
            *this->GetLogPrefix(*Task),
            *Task->Request.PartyId->ToString());
    }

    this->UnlockPartyIfNeeded(
        Task,
        FMatchmakingAlgorithmStepUnlockParty::FOnInternalPartyUnlocked::CreateSP(
            this,
            &FMatchmakingAlgorithmStepCompleteUnlockParty::OnComplete,
            TWeakPtr<FMatchmakingEngineTask>(Task)));
}

#endif

REDPOINT_EOS_CODE_GUARD_END()