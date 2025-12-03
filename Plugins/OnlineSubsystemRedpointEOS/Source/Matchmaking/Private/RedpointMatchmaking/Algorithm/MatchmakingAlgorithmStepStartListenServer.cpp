// Copyright June Rhodes. All Rights Reserved.

#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepStartListenServer.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "../MatchmakingSerialization.h"
#include "Engine/Engine.h"
#include "Engine/NetDriver.h"
#include "Engine/World.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepCreateSessionForListenServer.h"
#include "RedpointMatchmaking/MatchmakingConstants.h"
#include "RedpointMatchmaking/MatchmakingErrors.h"
#include "RedpointMatchmaking/MatchmakingLog.h"
#include "UObject/Object.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

TSharedPtr<FMatchmakingAlgorithmStepStateStartListenServer> FMatchmakingAlgorithmStepStartListenServer::CreateNewState()
{
    return MakeShared<FMatchmakingAlgorithmStepStateStartListenServer>();
}

void FMatchmakingAlgorithmStepStartListenServer::OnMapListening(
    const UWorld::FActorsInitializedParams &ActorInitParams,
    TWeakPtr<FMatchmakingEngineTask> TaskWk)
{
    TSharedPtr<FMatchmakingEngineTask> Task = TaskWk.Pin();
    if (!Task.IsValid())
    {
        return;
    }

    if (!Task->CurrentStep.IsEqual(MMSTEP_START_LISTEN_SERVER))
    {
        return;
    }

    auto State = this->GetState(*Task);

    UNetDriver *NetDriver = ActorInitParams.World->GetNetDriver();
    if (!IsValid(NetDriver))
    {
        this->PermanentError(*Task, OnlineMatchmaking::Errors::StartedMapDoesNotHaveNetDriver());
        FWorldDelegates::OnWorldInitializedActors.Remove(State->WorldInitHandle);
        State->WorldInitHandle.Reset();
        return;
    }

    if (NetDriver->GetNetMode() != ENetMode::NM_ListenServer)
    {
        this->PermanentError(*Task, OnlineMatchmaking::Errors::NetDriverInWrongMode());
        FWorldDelegates::OnWorldInitializedActors.Remove(State->WorldInitHandle);
        State->WorldInitHandle.Reset();
        return;
    }

    UE_LOG(LogMatchmaking, Verbose, TEXT("%sSuccessfully started listen server"), *this->GetLogPrefix(*Task));
    FWorldDelegates::OnWorldInitializedActors.Remove(State->WorldInitHandle);
    State->WorldInitHandle.Reset();

    this->MoveToStep(*Task, MMSTEP_CREATE_SESSION_FOR_LISTEN_SERVER);
}

void FMatchmakingAlgorithmStepStartListenServer::Start(TSharedRef<FMatchmakingEngineTask> Task)
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
        State->WorldInitHandle = FWorldDelegates::OnWorldInitializedActors.AddSP(
            this,
            &FMatchmakingAlgorithmStepStartListenServer::OnMapListening,
            TWeakPtr<FMatchmakingEngineTask>(Task));

        // Figure out the world context from the online subsystem.
        UE_LOG(LogMatchmaking, Verbose, TEXT("%sStarting listen server"), *this->GetLogPrefix(*Task));
        FWorldContext &CurrentWorldContext =
            GEngine->GetWorldContextFromHandleChecked(Task->Request.WorldContextHandle);
        GEngine->SetClientTravel(
            CurrentWorldContext.World(),
            *FString::Printf(TEXT("%s?listen"), *Task->Request.CompletionBehaviour.MapName.ToString()),
            ETravelType::TRAVEL_Absolute);
    }
}

#endif

REDPOINT_EOS_CODE_GUARD_END()