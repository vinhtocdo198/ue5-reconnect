// Copyright June Rhodes. All Rights Reserved.

#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepConnectToGameServer.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "Engine/Engine.h"
#include "Engine/NetDriver.h"
#include "Engine/World.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepCompleteMatchFireEvent.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepDeleteMatchmakingLobby.h"
#include "RedpointMatchmaking/MatchmakingErrors.h"
#include "RedpointMatchmaking/MatchmakingLog.h"
#include "UObject/Object.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

FMatchmakingAlgorithmStepStateConnectToGameServer::FMatchmakingAlgorithmStepStateConnectToGameServer()
    : WorldInitHandle()
    , bEventHandled(false)
{
}

TSharedPtr<FMatchmakingAlgorithmStepStateConnectToGameServer> FMatchmakingAlgorithmStepConnectToGameServer::
    CreateNewState()
{
    return MakeShared<FMatchmakingAlgorithmStepStateConnectToGameServer>();
}

void FMatchmakingAlgorithmStepConnectToGameServer::OnMapStart(
    const UWorld::FActorsInitializedParams &ActorInitParams,
    TWeakPtr<FMatchmakingEngineTask> TaskWk)
{
    TSharedPtr<FMatchmakingEngineTask> Task = TaskWk.Pin();
    if (!Task.IsValid())
    {
        return;
    }

    if (!Task->CurrentStep.IsEqual(MMSTEP_CONNECT_TO_GAME_SERVER))
    {
        return;
    }

    UE_LOG(
        LogMatchmaking,
        Verbose,
        TEXT("%sDetected map initialization after connection request"),
        *this->GetLogPrefix(*Task));

    auto State = this->GetState(*Task);
    if (State->bEventHandled)
    {
        // In some games, the FWorldDelegates::OnWorldInitializedActors event is called multiple times for a single
        // server join, and we only want to do this once.
        return;
    }
    State->bEventHandled = true;

    UNetDriver *NetDriver = ActorInitParams.World->GetNetDriver();
    if (!IsValid(NetDriver))
    {
        this->PermanentError(*Task, OnlineMatchmaking::Errors::StartedMapDoesNotHaveNetDriver());
        FWorldDelegates::OnWorldInitializedActors.Remove(State->WorldInitHandle);
        State->WorldInitHandle.Reset();
        return;
    }

    if (NetDriver->GetNetMode() != ENetMode::NM_Client)
    {
        this->PermanentError(*Task, OnlineMatchmaking::Errors::NetDriverInWrongMode());
        FWorldDelegates::OnWorldInitializedActors.Remove(State->WorldInitHandle);
        State->WorldInitHandle.Reset();
        return;
    }

    UE_LOG(LogMatchmaking, Verbose, TEXT("%sSuccessfully connected to game server"), *this->GetLogPrefix(*Task));
    FWorldDelegates::OnWorldInitializedActors.Remove(State->WorldInitHandle);
    State->WorldInitHandle.Reset();

    // Fire completion event on client.
    UE_LOG(
        LogMatchmaking,
        Verbose,
        TEXT("%sMatchmaking complete after connecting to game server"),
        *this->GetLogPrefix(*Task));
    if (Task->bResponsibleForLobbyDeletionStep)
    {
        this->MoveToStep(*Task, MMSTEP_DELETE_MATCHMAKING_LOBBY, MMSTEP_COMPLETE_MATCH_FIRE_EVENT);
    }
    else
    {
        this->MoveToStep(*Task, MMSTEP_COMPLETE_MATCH_FIRE_EVENT);
    }
}

void FMatchmakingAlgorithmStepConnectToGameServer::Start(TSharedRef<FMatchmakingEngineTask> Task, FString ConnectionUrl)
{
    if (GEngine == nullptr)
    {
        this->PermanentError(*Task, OnlineMatchmaking::Errors::GlobalEngineNotAvailable());
        return;
    }

    if (auto Pins = Task->PinAndContinue())
    {
        // Figure out the controller index for the request user.
        int ControllerIndex = -1;
        for (int i = 0; i < MAX_LOCAL_PLAYERS; i++)
        {
            if (Pins->OnlineIdentity->GetLoginStatus(i) != ELoginStatus::NotLoggedIn)
            {
                TSharedPtr<const FUniqueNetId> LocalUserId = Pins->OnlineIdentity->GetUniquePlayerId(i);
                if (LocalUserId.IsValid() && *LocalUserId == *Task->Request.UserId)
                {
                    ControllerIndex = i;
                    break;
                }
            }
        }
        if (ControllerIndex == -1)
        {
            this->PermanentError(*Task, OnlineMatchmaking::Errors::RequestUserIsNotLocallySignedIn());
            return;
        }

        // Set the controller index on the task, so we can refer to it in later steps.
        Task->ControllerIndex = ControllerIndex;

        // Listen for the map start.
        auto State = this->GetState(*Task);
        State->bEventHandled = false;
        State->WorldInitHandle = FWorldDelegates::OnWorldInitializedActors.AddSP(
            this,
            &FMatchmakingAlgorithmStepConnectToGameServer::OnMapStart,
            TWeakPtr<FMatchmakingEngineTask>(Task));

        // Figure out the world context from the online subsystem.
        UE_LOG(
            LogMatchmaking,
            Verbose,
            TEXT("%sConnecting to game server: %s"),
            *this->GetLogPrefix(*Task),
            *ConnectionUrl);
        FWorldContext &CurrentWorldContext =
            GEngine->GetWorldContextFromHandleChecked(Task->Request.WorldContextHandle);
        GEngine->SetClientTravel(CurrentWorldContext.World(), *ConnectionUrl, ETravelType::TRAVEL_Absolute);
    }
}

#endif

REDPOINT_EOS_CODE_GUARD_END()