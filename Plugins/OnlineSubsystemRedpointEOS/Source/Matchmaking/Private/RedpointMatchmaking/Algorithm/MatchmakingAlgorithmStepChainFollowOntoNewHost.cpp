// Copyright June Rhodes. All Rights Reserved.

#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepChainFollowOntoNewHost.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepFollowHost.h"
#include "RedpointMatchmaking/EventHandlers/MatchmakingEventHandlerClientFollowing.h"
#include "RedpointMatchmaking/MatchmakingConstants.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

TSharedPtr<FMatchmakingAlgorithmStepStateChainFollowOntoNewHost> FMatchmakingAlgorithmStepChainFollowOntoNewHost::
    CreateNewState()
{
    return MakeShared<FMatchmakingAlgorithmStepStateChainFollowOntoNewHost>();
}

void FMatchmakingAlgorithmStepChainFollowOntoNewHost::OnOldLobbyDisconnected(
    const FOnlineError &Error,
    const FUniqueNetId &UserId,
    TWeakPtr<FMatchmakingEngineTask> TaskWk,
    TSharedPtr<const FOnlineLobbyId> NewLobbyId,
    TSharedPtr<const FOnlineLobbyId> OldLobbyId)
{
    auto Task = TaskWk.Pin();
    if (!Task.IsValid())
    {
        return;
    }

    if (!Task->CurrentStep.IsEqual(MMSTEP_CHAIN_FOLLOW_ONTO_NEW_HOST))
    {
        return;
    }

    if (!Error.bSucceeded)
    {
        if (Error.ErrorCode.EndsWith(".timed_out") || Error.ErrorCode.EndsWith(".already_pending"))
        {
            if (auto Pins = Task->PinAndContinue())
            {
                // If we successfully start the DisconnectLobby operation, then return early and let
                // that retry run. If we can't start the DisconnectLobby operation, then we might have
                // actually disconnected and we can let matchmaking continue.
                if (Pins->OnlineLobby->DisconnectLobby(
                        *Task->Request.UserId,
                        *OldLobbyId,
                        FOnLobbyOperationComplete::CreateSP(
                            this,
                            &FMatchmakingAlgorithmStepChainFollowOntoNewHost::OnOldLobbyDisconnected,
                            TWeakPtr<FMatchmakingEngineTask>(Task),
                            MoveTemp(NewLobbyId),
                            OldLobbyId)))
                {
                    // The operation has been retried.
                    return;
                }
            }
        }
        else
        {
            this->RestartableError(
                *Task,
                FString::Printf(
                    TEXT("Unable to disconnect from old lobby as part of follow chain: %s"),
                    *Error.ToLogString()));
            return;
        }
    }

    EMIT_TASK_COUNTER_TRACE(Task);
    this->MoveToStep(*Task, MMSTEP_FOLLOW_HOST);
}

void FMatchmakingAlgorithmStepChainFollowOntoNewHost::OnNewLobbyConnected(
    const FOnlineError &Error,
    const FUniqueNetId &UserId,
    const TSharedPtr<class FOnlineLobby> &Lobby,
    TWeakPtr<FMatchmakingEngineTask> TaskWk,
    TSharedPtr<const FOnlineLobbyId> NewLobbyId,
    int AttemptNum)
{
    auto Task = TaskWk.Pin();
    if (!Task.IsValid())
    {
        return;
    }

    if (!Task->CurrentStep.IsEqual(MMSTEP_CHAIN_FOLLOW_ONTO_NEW_HOST))
    {
        return;
    }

    if (!Error.bSucceeded)
    {
        if (Error.ErrorCode.EndsWith(".too_many_players") && AttemptNum < 10)
        {
            // We couldn't connect because there were too many players. This could happen if another
            // unpermitted client connects to the lobby and takes up our slot, so we retry the connection
            // up to a certain limit.
            if (auto Pins = Task->PinAndContinue())
            {
                if (!Pins->OnlineLobby->ConnectLobby(
                        *Task->Request.UserId,
                        *NewLobbyId,
                        FOnLobbyCreateOrConnectComplete::CreateSP(
                            this,
                            &FMatchmakingAlgorithmStepChainFollowOntoNewHost::OnNewLobbyConnected,
                            TWeakPtr<FMatchmakingEngineTask>(Task),
                            NewLobbyId,
                            AttemptNum + 1)))
                {
                    this->RestartableError(
                        *Task,
                        FString::Printf(
                            TEXT("Unable to connect target lobby to follow as part of follow chain (attempt #%d)"),
                            AttemptNum + 1));
                }
                return;
            }
        }

        if (Error.ErrorCode.EndsWith(".timed_out") || Error.ErrorCode.EndsWith(".already_pending"))
        {
            // If we timeout, always retry.
            if (auto Pins = Task->PinAndContinue())
            {
                if (!Pins->OnlineLobby->ConnectLobby(
                        *Task->Request.UserId,
                        *NewLobbyId,
                        FOnLobbyCreateOrConnectComplete::CreateSP(
                            this,
                            &FMatchmakingAlgorithmStepChainFollowOntoNewHost::OnNewLobbyConnected,
                            TWeakPtr<FMatchmakingEngineTask>(Task),
                            NewLobbyId,
                            AttemptNum)))
                {
                    this->RestartableError(
                        *Task,
                        FString::Printf(
                            TEXT("Unable to connect target lobby to follow as part of follow chain (attempt #%d)"),
                            AttemptNum));
                }
                return;
            }
        }

        this->RestartableError(
            *Task,
            FString::Printf(
                TEXT("Unable to connect target lobby '%s' to follow as part of follow chain: %s"),
                *NewLobbyId->ToString(),
                *Error.ToLogString()));
        return;
    }

    if (auto Pins = Task->PinAndContinue())
    {
        // Swap over the active lobby ID
        auto OldMatchmakingLobbyId = Task->MatchmakingLobbyId;
        Task->MatchmakingLobbyId = NewLobbyId;

        if (!Pins->OnlineLobby->DisconnectLobby(
                *Task->Request.UserId,
                *OldMatchmakingLobbyId,
                FOnLobbyOperationComplete::CreateSP(
                    this,
                    &FMatchmakingAlgorithmStepChainFollowOntoNewHost::OnOldLobbyDisconnected,
                    TWeakPtr<FMatchmakingEngineTask>(Task),
                    NewLobbyId,
                    OldMatchmakingLobbyId)))
        {
            this->RestartableError(
                *Task,
                TEXT("Unable to disconnect from old lobby as part of follow chain (failed to start operation)"));
            return;
        }
    }
}

void FMatchmakingAlgorithmStepChainFollowOntoNewHost::Start(TSharedRef<FMatchmakingEngineTask> Task)
{
    if (auto Pins = Task->PinAndContinue())
    {
        FVariantData NewLobbyIdData;
        if (Pins->OnlineLobby->GetLobbyMetadataValue(
                *Task->Request.UserId,
                *Task->MatchmakingLobbyId,
                MatchmakingMetadataFollowTargetLobbyId,
                NewLobbyIdData))
        {
            if (NewLobbyIdData.GetType() == EOnlineKeyValuePairDataType::String)
            {
                FString NewLobbyIdString;
                NewLobbyIdData.GetValue(NewLobbyIdString);
                TSharedPtr<const FOnlineLobbyId> NewLobbyId =
                    Pins->OnlineLobby->ParseSerializedLobbyId(NewLobbyIdString);
                if (NewLobbyId.IsValid())
                {
                    if (!Pins->OnlineLobby->ConnectLobby(
                            *Task->Request.UserId,
                            *NewLobbyId,
                            FOnLobbyCreateOrConnectComplete::CreateSP(
                                this,
                                &FMatchmakingAlgorithmStepChainFollowOntoNewHost::OnNewLobbyConnected,
                                TWeakPtr<FMatchmakingEngineTask>(Task),
                                NewLobbyId,
                                0)))
                    {
                        this->RestartableError(
                            *Task,
                            FString::Printf(
                                TEXT("Unable to connect target lobby to follow as part of follow chain (attempt #%d)"),
                                1));
                    }

                    return;
                }
            }
        }

        this->RestartableError(*Task, TEXT("Unable to get target lobby to follow as part of follow chain"));
    }
}

#endif

REDPOINT_EOS_CODE_GUARD_END()