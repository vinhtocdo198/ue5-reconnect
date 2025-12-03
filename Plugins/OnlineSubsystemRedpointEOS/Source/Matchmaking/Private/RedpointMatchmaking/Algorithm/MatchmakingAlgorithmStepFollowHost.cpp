// Copyright June Rhodes. All Rights Reserved.

#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepFollowHost.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "Containers/Ticker.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepAbandonStart.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepChainFollowOntoNewHost.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepCompleteMatchAsClient.h"
#include "RedpointMatchmaking/EventHandlers/MatchmakingEventHandlerAbandonment.h"
#include "RedpointMatchmaking/EventHandlers/MatchmakingEventHandlerClientFollowing.h"
#include "RedpointMatchmaking/EventHandlers/MatchmakingEventHandlerHostHeartbeat.h"
#include "RedpointMatchmaking/MatchmakingConstants.h"
#include "RedpointMatchmaking/MatchmakingLog.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

TSharedPtr<FMatchmakingAlgorithmStepStateFollowHost> FMatchmakingAlgorithmStepFollowHost::CreateNewState()
{
    return MakeShared<FMatchmakingAlgorithmStepStateFollowHost>();
}

bool FMatchmakingAlgorithmStepFollowHost::OnHeartbeatLost(
    float DeltaSeconds,
    TWeakPtr<FMatchmakingEngineTask> WeakTask)
{
    TSharedPtr<FMatchmakingEngineTask> Task = WeakTask.Pin();
    if (!Task.IsValid())
    {
        return false;
    }

    if (!Task->CurrentStep.IsEqual(MMSTEP_FOLLOW_HOST))
    {
        return false;
    }

    TSharedPtr<FMatchmakingAlgorithmStepStateFollowHost> State = this->GetState(*Task);
    if (!State->LastHeartbeatTimestamp.IsSet())
    {
        UE_LOG(
            LogMatchmaking,
            Warning,
            TEXT("Task %s: Never received a heartbeat within the timeout of %f seconds for matchmaking lobby '%s'."),
            *Task->TaskId,
            static_cast<double>(MatchmakingTimingHeartbeatTimeout),
            *Task->MatchmakingLobbyId->ToString());
    }
    else
    {
        auto DurationSinceHeartbeat =
            (FDateTime::UtcNow() - State->LastHeartbeatTimestamp.GetValue()).GetTotalSeconds();

        UE_LOG(
            LogMatchmaking,
            Verbose,
            TEXT("Task %s: Heartbeat lost callback is firing with %f seconds since last heartbeat (timeout is %f "
                 "seconds), for matchmaking lobby '%s'."),
            *Task->TaskId,
            DurationSinceHeartbeat,
            static_cast<double>(MatchmakingTimingHeartbeatTimeout),
            *Task->MatchmakingLobbyId->ToString());
    }

    EMIT_TASK_COUNTER_TRACE(Task);
    this->RestartableError(
        *Task,
        FString::Printf(
            TEXT("The lobby host stopped responding. Expected to get a heartbeat for lobby '%s' within %f seconds."),
            *Task->MatchmakingLobbyId->ToString(),
            MatchmakingTimingHeartbeatTimeout));
    return false;
}

void FMatchmakingAlgorithmStepFollowHost::Start(TSharedRef<FMatchmakingEngineTask> Task)
{
    // If we are a host and we are now following another host as a client, then by the time FOLLOW_HOST starts, the
    // target lobby might already be finalizing state. This is because we send the acceptance request and join their
    // lobby before we kick off FOLLOW_HOST, and thus the target lobby we are now following may see it's teams filled
    // and kick off completion prior to this code running. In this case, we need to detect if the lobby we are now
    // following is already finalizing and immediately move to COMPLETE_MATCH_AS_CLIENT since we will have missed the
    // CLIENT_FOLLOWED_LOBBY_COMPLETED_MATCH interrupt.
    //
    // This "missed event" check also applies if the target lobby has already started following another lobby.
    if (auto Pins = Task->PinAndContinue())
    {
        EMIT_TASK_COUNTER_TRACE(Task);
        FVariantData LobbyStatusData;
        if (Pins->OnlineLobby->GetLobbyMetadataValue(
                *Task->Request.UserId,
                *Task->MatchmakingLobbyId,
                MatchmakingMetadataLobbyStatus,
                LobbyStatusData))
        {
            if (LobbyStatusData.GetType() == EOnlineKeyValuePairDataType::String)
            {
                FString LobbyStatus;
                LobbyStatusData.GetValue(LobbyStatus);
                if (LobbyStatus == MatchmakingStatusFollowing)
                {
                    UE_LOG(
                        LogMatchmaking,
                        VeryVerbose,
                        TEXT("%sDetected follow chain as we begun to follow the host in matchmaking lobby %s"),
                        *this->GetLogPrefix(*Task),
                        *Task->MatchmakingLobbyId->ToString());
                    EMIT_TASK_COUNTER_TRACE(Task);
                    this->MoveToStep(*Task, MMSTEP_CHAIN_FOLLOW_ONTO_NEW_HOST);
                    return;
                }
                else if (LobbyStatus == MatchmakingStatusFinalizing)
                {
                    UE_LOG(
                        LogMatchmaking,
                        Verbose,
                        TEXT("%sDetected match finalization as we begun to follow the host in matchmaking lobby %s"),
                        *this->GetLogPrefix(*Task),
                        *Task->MatchmakingLobbyId->ToString());
                    EMIT_TASK_COUNTER_TRACE(Task);
                    this->MoveToStep(*Task, MMSTEP_COMPLETE_MATCH_AS_CLIENT);
                    return;
                }
            }
        }
        EMIT_TASK_COUNTER_TRACE(Task);
    }

    // Start our "heartbeat lost" timer. If this timer ever fires, then the client considers the host lost and
    // fails with a critical error.
    EMIT_TASK_COUNTER_TRACE(Task);
    TSharedPtr<FMatchmakingAlgorithmStepStateFollowHost> State = this->GetState(*Task);
    State->LostHeartbeatHandle = FTSTicker::GetCoreTicker().AddTicker(
        FTickerDelegate::CreateSP(
            this,
            &FMatchmakingAlgorithmStepFollowHost::OnHeartbeatLost,
            TWeakPtr<FMatchmakingEngineTask>(Task)),
        MatchmakingTimingHeartbeatTimeout);
    EMIT_TASK_COUNTER_TRACE(Task);
}

bool FMatchmakingAlgorithmStepFollowHost::CanHandleInterrupt(
    const TSharedRef<FMatchmakingEngineTask> &Task,
    FName InterruptType)
{
    return InterruptType.IsEqual(MMINTERRUPT_CLIENT_FOLLOWED_LOBBY_NOW_FOLLOWING_ANOTHER_LOBBY) ||
           InterruptType.IsEqual(MMINTERRUPT_CLIENT_FOLLOWED_LOBBY_COMPLETED_MATCH) ||
           InterruptType.IsEqual(MMINTERRUPT_CLIENT_GOT_HOST_HEARTBEAT) ||
           InterruptType.IsEqual(MMINTERRUPT_HAS_SPECIAL_HANDLING_OF_ABANDONMENT);
}

bool FMatchmakingAlgorithmStepFollowHost::HandleInterrupt(
    const TSharedRef<FMatchmakingEngineTask> &Task,
    FName InterruptType)
{
    if (InterruptType.IsEqual(MMINTERRUPT_CLIENT_FOLLOWED_LOBBY_NOW_FOLLOWING_ANOTHER_LOBBY))
    {
        UE_LOG(
            LogMatchmaking,
            VeryVerbose,
            TEXT("%sReceived notification that we need to chain onto a new host while following host in matchmaking "
                 "lobby %s"),
            *this->GetLogPrefix(*Task),
            *Task->MatchmakingLobbyId->ToString());
        TSharedPtr<FMatchmakingAlgorithmStepStateFollowHost> State = this->GetState(*Task);
        FTSTicker::GetCoreTicker().RemoveTicker(State->LostHeartbeatHandle);
        State->LostHeartbeatHandle.Reset();
        EMIT_TASK_COUNTER_TRACE(Task);
        this->MoveToStep(*Task, MMSTEP_CHAIN_FOLLOW_ONTO_NEW_HOST);
        return true;
    }

    if (InterruptType.IsEqual(MMINTERRUPT_CLIENT_FOLLOWED_LOBBY_COMPLETED_MATCH))
    {
        UE_LOG(
            LogMatchmaking,
            Verbose,
            TEXT("%sReceived notification that the match is finalizing while following host in matchmaking lobby %s"),
            *this->GetLogPrefix(*Task),
            *Task->MatchmakingLobbyId->ToString());
        EMIT_TASK_COUNTER_TRACE(Task);
        TSharedPtr<FMatchmakingAlgorithmStepStateFollowHost> State = this->GetState(*Task);
        FTSTicker::GetCoreTicker().RemoveTicker(State->LostHeartbeatHandle);
        State->LostHeartbeatHandle.Reset();
        EMIT_TASK_COUNTER_TRACE(Task);
        this->MoveToStep(*Task, MMSTEP_COMPLETE_MATCH_AS_CLIENT);
        return true;
    }

    if (InterruptType.IsEqual(MMINTERRUPT_CLIENT_GOT_HOST_HEARTBEAT))
    {
        UE_LOG(
            LogMatchmaking,
            VeryVerbose,
            TEXT("Task %s: Received heartbeat from matchmaking lobby %s"),
            *Task->TaskId,
            *Task->MatchmakingLobbyId->ToString());
        TSharedPtr<FMatchmakingAlgorithmStepStateFollowHost> State = this->GetState(*Task);
        EMIT_TASK_COUNTER_TRACE(Task);
        State->LastHeartbeatTimestamp = FDateTime::UtcNow();
        FTSTicker::GetCoreTicker().RemoveTicker(State->LostHeartbeatHandle);
        State->LostHeartbeatHandle.Reset();
        State->LostHeartbeatHandle = FTSTicker::GetCoreTicker().AddTicker(
            FTickerDelegate::CreateSP(
                this,
                &FMatchmakingAlgorithmStepFollowHost::OnHeartbeatLost,
                TWeakPtr<FMatchmakingEngineTask>(Task)),
            MatchmakingTimingHeartbeatTimeout);
        EMIT_TASK_COUNTER_TRACE(Task);
        return true;
    }

    if (InterruptType.IsEqual(MMINTERRUPT_ABANDON_CANCEL) || InterruptType.IsEqual(MMINTERRUPT_ABANDON_RESTART))
    {
        UE_LOG(
            LogMatchmaking,
            VeryVerbose,
            TEXT("Task %s: Clearing heartbeat lost handle before going into abandonment"),
            *Task->TaskId,
            *Task->MatchmakingLobbyId->ToString());
        EMIT_TASK_COUNTER_TRACE(Task);
        TSharedPtr<FMatchmakingAlgorithmStepStateFollowHost> State = this->GetState(*Task);
        FTSTicker::GetCoreTicker().RemoveTicker(State->LostHeartbeatHandle);
        State->LostHeartbeatHandle.Reset();
        EMIT_TASK_COUNTER_TRACE(Task);

        // We're not handling the abandonment, tell the event handler it still needs to move to the abandon step.
        return false;
    }

    return false;
}

#endif

REDPOINT_EOS_CODE_GUARD_END()