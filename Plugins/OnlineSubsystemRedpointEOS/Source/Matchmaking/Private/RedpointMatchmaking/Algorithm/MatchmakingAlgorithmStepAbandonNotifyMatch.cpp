// Copyright June Rhodes. All Rights Reserved.

#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepAbandonNotifyMatch.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepAbandonFinish.h"
#include "RedpointMatchmaking/EventHandlers/MatchmakingEventHandlerAbandonment.h"
#include "RedpointMatchmaking/MatchmakingConstants.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

TSharedPtr<FMatchmakingAlgorithmStepStateAbandonNotifyMatch> FMatchmakingAlgorithmStepAbandonNotifyMatch::
    CreateNewState()
{
    return MakeShared<FMatchmakingAlgorithmStepStateAbandonNotifyMatch>();
}

void FMatchmakingAlgorithmStepAbandonNotifyMatch::OnDisconnectedFromFollowRequest(
    const FOnlineError &Error,
    const FUniqueNetId &UserId,
    TWeakPtr<FMatchmakingEngineTask> TaskWk,
    EMatchmakingAbandonmentType AbandonmentType)
{
    auto Task = TaskWk.Pin();
    check(Task.IsValid());

    check(Task->CurrentStep.IsEqual(MMSTEP_ABANDON_NOTIFY_MATCH));

    if (!Error.bSucceeded)
    {
        if (Error.ErrorCode.EndsWith(".timed_out") || Error.ErrorCode.EndsWith(".already_pending"))
        {
            if (auto Pins = Task->PinAndContinue())
            {
                if (Task->MatchmakingLobbyId.IsValid())
                {
                    if (Pins->OnlineLobby->DisconnectLobby(
                            *Task->Request.UserId,
                            *Task->MatchmakingLobbyId,
                            FOnLobbyOperationComplete::CreateSP(
                                this,
                                &FMatchmakingAlgorithmStepAbandonNotifyMatch::OnDisconnectedFromFollowRequest,
                                TWeakPtr<FMatchmakingEngineTask>(Task),
                                AbandonmentType)))
                    {
                        return;
                    }
                }
            }
        }

        UE_LOG(LogMatchmaking, Warning, TEXT("Unable to disconnect from follow request lobby"), *Error.ToLogString());
        // Fallthrough to MoveToStep below.
    }

    this->MoveToStep(*Task, MMSTEP_ABANDON_FINISH, AbandonmentType);
    return;
}

void FMatchmakingAlgorithmStepAbandonNotifyMatch::OnDisconnectedFromMatch(
    const FOnlineError &Error,
    const FUniqueNetId &UserId,
    TWeakPtr<FMatchmakingEngineTask> TaskWk,
    EMatchmakingAbandonmentType AbandonmentType)
{
    auto Task = TaskWk.Pin();
    check(Task.IsValid());

    check(Task->CurrentStep.IsEqual(MMSTEP_ABANDON_NOTIFY_MATCH));

    if (!Error.bSucceeded)
    {
        if (Error.ErrorCode.EndsWith(".timed_out") || Error.ErrorCode.EndsWith(".already_pending"))
        {
            if (auto Pins = Task->PinAndContinue())
            {
                if (Task->MatchmakingLobbyId.IsValid())
                {
                    if (Pins->OnlineLobby->DisconnectLobby(
                            *Task->Request.UserId,
                            *Task->MatchmakingLobbyId,
                            FOnLobbyOperationComplete::CreateSP(
                                this,
                                &FMatchmakingAlgorithmStepAbandonNotifyMatch::OnDisconnectedFromMatch,
                                TWeakPtr<FMatchmakingEngineTask>(Task),
                                AbandonmentType)))
                    {
                        return;
                    }
                }
            }
        }

        UE_LOG(LogMatchmaking, Warning, TEXT("Unable to disconnect from matchmaking lobby"), *Error.ToLogString());
        // Fallthrough to leaving the follow request lobby below.
    }

    if (auto Pins = Task->PinAndContinue())
    {
        if (Task->MatchmakingFollowRequestLobbyId.IsValid())
        {
            if (Pins->OnlineLobby->DisconnectLobby(
                    *Task->Request.UserId,
                    *Task->MatchmakingFollowRequestLobbyId,
                    FOnLobbyOperationComplete::CreateSP(
                        this,
                        &FMatchmakingAlgorithmStepAbandonNotifyMatch::OnDisconnectedFromFollowRequest,
                        TWeakPtr<FMatchmakingEngineTask>(Task),
                        AbandonmentType)))
            {
                return;
            }
        }
        else
        {
            this->MoveToStep(*Task, MMSTEP_ABANDON_FINISH, AbandonmentType);
            return;
        }
    }

    UE_LOG(
        LogMatchmaking,
        Warning,
        TEXT("Unable to clean up follow request lobby properly when abandoning matchmaking."));
    this->MoveToStep(*Task, MMSTEP_ABANDON_FINISH, AbandonmentType);
    return;
}

void FMatchmakingAlgorithmStepAbandonNotifyMatch::OnMatchMembersNotified(
    const FOnlineError &Error,
    const FUniqueNetId &UserId,
    TWeakPtr<FMatchmakingEngineTask> TaskWk,
    EMatchmakingAbandonmentType AbandonmentType)
{
    auto Task = TaskWk.Pin();
    check(Task.IsValid());

    check(Task->CurrentStep.IsEqual(MMSTEP_ABANDON_NOTIFY_MATCH));

    if (!Error.bSucceeded)
    {
        if (Error.ErrorCode.EndsWith(".timed_out") || Error.ErrorCode.EndsWith(".already_pending"))
        {
            // Just use Start to retry the operation.
            this->Start(Task.ToSharedRef(), AbandonmentType);
            return;
        }

        UE_LOG(
            LogMatchmaking,
            Warning,
            TEXT("Unable to notify other match members that we are abandoning matchmaking: %s"),
            *Error.ToLogString());
        // Fallthrough to leaving the lobby below.
    }

    if (auto Pins = Task->PinAndContinue())
    {
        if (Task->MatchmakingLobbyId.IsValid())
        {
            if (Pins->OnlineLobby->DisconnectLobby(
                    *Task->Request.UserId,
                    *Task->MatchmakingLobbyId,
                    FOnLobbyOperationComplete::CreateSP(
                        this,
                        &FMatchmakingAlgorithmStepAbandonNotifyMatch::OnDisconnectedFromMatch,
                        TWeakPtr<FMatchmakingEngineTask>(Task),
                        AbandonmentType)))
            {
                return;
            }
        }
        else
        {
            this->MoveToStep(*Task, MMSTEP_ABANDON_FINISH, AbandonmentType);
            return;
        }
    }

    UE_LOG(LogMatchmaking, Warning, TEXT("Unable to clean up lobby connections properly when abandoning matchmaking."));
    this->MoveToStep(*Task, MMSTEP_ABANDON_FINISH, AbandonmentType);
    return;
}

void FMatchmakingAlgorithmStepAbandonNotifyMatch::Start(
    TSharedRef<FMatchmakingEngineTask> Task,
    EMatchmakingAbandonmentType AbandonmentType)
{
    if (auto Pins = Task->PinAndContinue())
    {
        if (!Task->MatchmakingLobbyId.IsValid())
        {
            this->MoveToStep(*Task, MMSTEP_ABANDON_FINISH, AbandonmentType);
            return;
        }

        auto Txn = Pins->OnlineLobby->MakeUpdateLobbyMemberTransaction(
            *Task->Request.UserId,
            *Task->MatchmakingLobbyId,
            *Task->Request.UserId);
        Txn->SetMetadata.Add(MatchmakingAbandonKey, FVariantData(FString(MatchmakingAbandonMatch)));

        if (!Pins->OnlineLobby->UpdateMemberSelf(
                *Task->Request.UserId,
                *Task->MatchmakingLobbyId,
                *Txn,
                FOnLobbyOperationComplete::CreateSP(
                    this,
                    &FMatchmakingAlgorithmStepAbandonNotifyMatch::OnMatchMembersNotified,
                    TWeakPtr<FMatchmakingEngineTask>(Task),
                    AbandonmentType)))
        {
            UE_LOG(
                LogMatchmaking,
                Warning,
                TEXT("Unable to notify match members that are are abandoning matchmaking."));
            this->MoveToStep(*Task, MMSTEP_ABANDON_FINISH, AbandonmentType);
            return;
        }
    }
}

bool FMatchmakingAlgorithmStepAbandonNotifyMatch::CanHandleInterrupt(
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

bool FMatchmakingAlgorithmStepAbandonNotifyMatch::HandleInterrupt(
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