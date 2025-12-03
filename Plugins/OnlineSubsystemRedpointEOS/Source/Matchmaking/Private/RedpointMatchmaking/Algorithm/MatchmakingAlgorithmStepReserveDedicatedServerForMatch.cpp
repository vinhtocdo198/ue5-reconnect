// Copyright June Rhodes. All Rights Reserved.

#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepReserveDedicatedServerForMatch.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "DedicatedServerMatchmakingBeaconClient.h"
#include "Engine/Engine.h"
#include "Engine/NetConnection.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepConnectToGameServer.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepConnectionInformationSentForDedicatedServer.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepFindDedicatedServerSession.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepJoinServerSession.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepSendSessionIdOrConnectionString.h"
#include "RedpointMatchmaking/EventHandlers/MatchmakingEventHandlerAbandonment.h"
#include "RedpointMatchmaking/MatchmakingConstants.h"
#include "RedpointMatchmaking/MatchmakingErrors.h"
#include "RedpointMatchmaking/MatchmakingLog.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

TSharedPtr<FMatchmakingAlgorithmStepStateReserveDedicatedServerForMatch>
FMatchmakingAlgorithmStepReserveDedicatedServerForMatch::CreateNewState()
{
    return MakeShared<FMatchmakingAlgorithmStepStateReserveDedicatedServerForMatch>();
}

void FMatchmakingAlgorithmStepReserveDedicatedServerForMatch::DestroyAndCleanupBeacon(FMatchmakingEngineTask &Task)
{
    auto State = this->GetState(Task);
    if (State->BeaconActor.IsValid())
    {
        State->BeaconActor->OnConnectedDelegate.Unbind();
        State->BeaconActor->OnFailureDelegate.Unbind();
        State->BeaconActor->OnHostConnectionFailure().Unbind();
        State->BeaconActor->DestroyBeacon();
        State->BeaconActor.Reset();
    }
}

void FMatchmakingAlgorithmStepReserveDedicatedServerForMatch::OnConnected(
    TWeakPtr<FMatchmakingEngineTask> TaskWk,
    TSoftObjectPtr<class ADedicatedServerMatchmakingBeaconClient> BeaconWk,
    FOnlineSessionSearchResult SearchResult)
{
    auto Task = TaskWk.Pin();
    check(Task.IsValid());

    UE_LOG(
        LogMatchmaking,
        Verbose,
        TEXT("%sBeacon client successfully connected to host."),
        *this->GetLogPrefix(*Task));

    EMIT_TASK_COUNTER_TRACE(Task);

    if (!Task->CurrentStep.IsEqual(MMSTEP_RESERVE_DEDICATED_SERVER_FOR_MATCH))
    {
        return;
    }

    if (auto Pins = Task->PinAndContinue())
    {
        if (!BeaconWk.IsValid())
        {
            this->RestartableError(*Task, TEXT("Unable to reserve dedicated server: Beacon actor has disappeared"));
            return;
        }

        UE_LOG(
            LogMatchmaking,
            Verbose,
            TEXT("%sSuccessfully connected to dedicated server with session ID %s, attempting to reserve..."),
            *this->GetLogPrefix(*Task),
            *SearchResult.GetSessionIdStr());

        TArray<FDedicatedServerMatchmakingBeaconClientMatchMember> MatchMembers;
        TArray<int32> TeamSizes;
        for (int32 i = 0; i < Task->StoredResponse->Teams.Num(); i++)
        {
            const auto &Team = Task->StoredResponse->Teams[i];
            TeamSizes.Add(Team.Slots.Num());

            for (int32 s = 0; s < Team.Slots.Num(); s++)
            {
                const auto &Slot = Team.Slots[s];

                FDedicatedServerMatchmakingBeaconClientMatchMember Member;
                if (Slot.Type == EMatchmakingEngineResponseTeamSlotType::Empty)
                {
                    Member.UserId = FUniqueNetIdRepl();
                    Member.Team = i;
                    Member.Slot = s;
                    Member.SlotType = 0;
                }
                else if (Slot.Type == EMatchmakingEngineResponseTeamSlotType::AI)
                {
                    Member.UserId = FUniqueNetIdRepl();
                    Member.Team = i;
                    Member.Slot = s;
                    Member.SlotType = 1;
                }
                else if (Slot.Type == EMatchmakingEngineResponseTeamSlotType::User)
                {
                    Member.UserId = FUniqueNetIdRepl(*Slot.UserId);
                    Member.Team = i;
                    Member.Slot = s;
                    Member.SlotType = 2;
                }
                MatchMembers.Add(Member);
            }
        }

        BeaconWk->OnReserveForMatchResultDelegate.BindSP(
            this,
            &FMatchmakingAlgorithmStepReserveDedicatedServerForMatch::OnReserveForMatchResult,
            TWeakPtr<FMatchmakingEngineTask>(Task),
            BeaconWk,
            SearchResult);
        BeaconWk->ReserveForMatch(MatchMembers, TeamSizes);
    }
}

void FMatchmakingAlgorithmStepReserveDedicatedServerForMatch::OnReserveForMatchResult(
    bool bWasSuccessful,
    TWeakPtr<FMatchmakingEngineTask> TaskWk,
    TSoftObjectPtr<class ADedicatedServerMatchmakingBeaconClient> BeaconWk,
    FOnlineSessionSearchResult SearchResult)
{
    auto Task = TaskWk.Pin();
    check(Task.IsValid());

    if (!Task->CurrentStep.IsEqual(MMSTEP_RESERVE_DEDICATED_SERVER_FOR_MATCH))
    {
        return;
    }

    EMIT_TASK_COUNTER_TRACE(Task);

    if (auto Pins = Task->PinAndContinue())
    {
        if (!BeaconWk.IsValid())
        {
            this->RestartableError(*Task, TEXT("Unable to reserve dedicated server: Beacon actor has disappeared"));
            return;
        }

        if (!bWasSuccessful)
        {
            UE_LOG(
                LogMatchmaking,
                Verbose,
                TEXT("%sDedicated server %s denied our reservation request, destroying beacon actor..."),
                *this->GetLogPrefix(*Task),
                *SearchResult.GetSessionIdStr());

            this->DestroyAndCleanupBeacon(*Task);

            // Return to searching.
            Task->ExcludedDedicatedServerSessionIds.Add(SearchResult.GetSessionIdStr());
            this->MoveToStep(*Task, MMSTEP_FIND_DEDICATED_SERVER_SESSION);
            return;
        }

        // We've successfully reserved this dedicated server for our match. Tell the other match members
        // what session ID to join, and then join ourselves.

        FString ConnectionKey;
        FString ConnectionValue;

        auto Txn = Pins->OnlineLobby->MakeUpdateLobbyTransaction(*Task->Request.UserId, *Task->MatchmakingLobbyId);

        if (Task->Request.CompletionBehaviour.bConnectViaSessionID)
        {
            UE_LOG(
                LogMatchmaking,
                Verbose,
                TEXT("%sSending session ID '%s' to other match members"),
                *this->GetLogPrefix(*Task),
                *SearchResult.GetSessionIdStr());
            ConnectionKey = MatchmakingMetadataConnectionSessionId;
            ConnectionValue = SearchResult.GetSessionIdStr();
        }
        else
        {
            FString ConnectInfo;
            verifyf(
                Pins->OnlineSession->GetResolvedConnectString(SearchResult, NAME_GamePort, ConnectInfo),
                TEXT("Expected GetResolvedConnectString to return a connection string in OnReserveForMatchResult"));
            UE_LOG(
                LogMatchmaking,
                Verbose,
                TEXT("%sSending connection URL '%s' to other match members"),
                *this->GetLogPrefix(*Task),
                *ConnectInfo);
            ConnectionKey = MatchmakingMetadataConnectionUrl;
            ConnectionValue = ConnectInfo;
        }

        // Destroy the beacon before we change steps.
        this->DestroyAndCleanupBeacon(*Task);

        Task->EventData.Add(
            TEXT("dedicated.next_step"),
            Task->Request.CompletionBehaviour.bConnectViaSessionID ? (MMSTEP_JOIN_SERVER_SESSION).ToString()
                                                                   : (MMSTEP_CONNECT_TO_GAME_SERVER).ToString());
        Task->EventData.Add(TEXT("dedicated.connection_info"), ConnectionValue);

        this->MoveToStep(
            *Task,
            MMSTEP_SEND_SESSION_ID_OR_CONNECTION_STRING,
            MMSTEP_CONNECTION_INFORMATION_SENT_FOR_DEDICATED_SERVER,
            ConnectionKey,
            ConnectionValue);
    }
}

void FMatchmakingAlgorithmStepReserveDedicatedServerForMatch::OnHostConnectionFailure(
    TWeakPtr<FMatchmakingEngineTask> TaskWk,
    TSoftObjectPtr<class ADedicatedServerMatchmakingBeaconClient> BeaconWk,
    FOnlineSessionSearchResult SearchResult)
{
    auto Task = TaskWk.Pin();
    check(Task.IsValid());

    UE_LOG(LogMatchmaking, Verbose, TEXT("%sBeacon client failed to connect to host."), *this->GetLogPrefix(*Task));

    EMIT_TASK_COUNTER_TRACE(Task);

    if (!Task->CurrentStep.IsEqual(MMSTEP_RESERVE_DEDICATED_SERVER_FOR_MATCH))
    {
        return;
    }

    if (auto Pins = Task->PinAndContinue())
    {
        UE_LOG(
            LogMatchmaking,
            Error,
            TEXT("%sFailed to connect to dedicated server with session ID %s."),
            *this->GetLogPrefix(*Task),
            *SearchResult.GetSessionIdStr());

        if (!Pins->OnlineSession.IsValid())
        {
            this->RestartableError(*Task, TEXT("Unable to reserve dedicated server: Session interface is not valid"));
            return;
        }

        this->DestroyAndCleanupBeacon(*Task);

        // Return to searching.
        Task->ExcludedDedicatedServerSessionIds.Add(SearchResult.GetSessionIdStr());
        this->MoveToStep(*Task, MMSTEP_FIND_DEDICATED_SERVER_SESSION);
        return;
    }
}

void FMatchmakingAlgorithmStepReserveDedicatedServerForMatch::Start(
    TSharedRef<FMatchmakingEngineTask> Task,
    FOnlineSessionSearchResult SearchResult)
{
    if (auto Pins = Task->PinAndContinue())
    {
        if (!Pins->OnlineSession.IsValid())
        {
            this->RestartableError(*Task, TEXT("Unable to reserve dedicated server: Session interface is not valid"));
            return;
        }

        FName Port = Task->Request.CompletionBehaviour.DedicatedServerBeaconPort;
        if (Port.IsNone())
        {
            Port = FName(*FString::Printf(TEXT("%d"), MatchmakingDedicatedServerDefaultPort));
        }

        UE_LOG(
            LogMatchmaking,
            Verbose,
            TEXT("%sGetting resolved connection string for dedicated server session %s for beacon port %s."),
            *this->GetLogPrefix(*Task),
            *SearchResult.GetSessionIdStr(),
            *Port.ToString());

        FString ConnectInfo;
        if (!Pins->OnlineSession->GetResolvedConnectString(SearchResult, Port, ConnectInfo))
        {
            // This dedicated server doesn't have the required connection string available. Exclude it and go back to
            // searching.
            UE_LOG(
                LogMatchmaking,
                Verbose,
                TEXT("%sDedicated server %s did not have a connection string for beacon port %s."),
                *this->GetLogPrefix(*Task),
                *SearchResult.GetSessionIdStr(),
                *Port.ToString());
            Task->ExcludedDedicatedServerSessionIds.Add(SearchResult.GetSessionIdStr());
            this->MoveToStep(*Task, MMSTEP_FIND_DEDICATED_SERVER_SESSION);
            return;
        }

        UE_LOG(
            LogMatchmaking,
            Verbose,
            TEXT("%sUsing connection string %s for beacon connection."),
            *this->GetLogPrefix(*Task),
            *ConnectInfo);

        // Get the world context for spawning the beacon.
        if (GEngine == nullptr)
        {
            this->RestartableError(*Task, TEXT("Unable to reserve dedicated server: GEngine is nullptr"));
            return;
        }

        FWorldContext *WorldContext = GEngine->GetWorldContextFromHandle(Task->Request.WorldContextHandle);
        if (WorldContext == nullptr)
        {
            this->RestartableError(*Task, TEXT("Unable to reserve dedicated server: World context could not be found"));
            return;
        }

        // Get the world from the world context.
        UWorld *World = WorldContext->World();
        if (!IsValid(World))
        {
            this->RestartableError(
                *Task,
                TEXT("Unable to reserve dedicated server: World of the world context is invalid"));
            return;
        }

        // Create the client beacon to talk to the dedicated server.
        auto State = this->GetState(*Task);
        if (State->BeaconActor.IsValid())
        {
            this->DestroyAndCleanupBeacon(*Task);
        }
        ADedicatedServerMatchmakingBeaconClient *Beacon = World->SpawnActor<ADedicatedServerMatchmakingBeaconClient>();
        State->BeaconActor = Beacon;
        if (!IsValid(Beacon))
        {
            this->RestartableError(
                *Task,
                TEXT("Unable to reserve dedicated server: Unable to spawn beacon client for reserving dedicated "
                     "server"));
            return;
        }

        Beacon->OnConnectedDelegate.BindSP(
            this,
            &FMatchmakingAlgorithmStepReserveDedicatedServerForMatch::OnConnected,
            TWeakPtr<FMatchmakingEngineTask>(Task),
            TSoftObjectPtr<ADedicatedServerMatchmakingBeaconClient>(Beacon),
            SearchResult);
        Beacon->OnFailureDelegate.BindSP(
            this,
            &FMatchmakingAlgorithmStepReserveDedicatedServerForMatch::OnHostConnectionFailure,
            TWeakPtr<FMatchmakingEngineTask>(Task),
            TSoftObjectPtr<ADedicatedServerMatchmakingBeaconClient>(Beacon),
            SearchResult);
        Beacon->OnHostConnectionFailure().BindSP(
            this,
            &FMatchmakingAlgorithmStepReserveDedicatedServerForMatch::OnHostConnectionFailure,
            TWeakPtr<FMatchmakingEngineTask>(Task),
            TSoftObjectPtr<ADedicatedServerMatchmakingBeaconClient>(Beacon),
            SearchResult);

        FURL OldURL;
        FURL URL(&OldURL, *ConnectInfo, ETravelType::TRAVEL_Absolute);

        UE_LOG(LogMatchmaking, Verbose, TEXT("%sInitializing beacon client..."), *this->GetLogPrefix(*Task));

        if (!Beacon->InitClient(URL))
        {
            this->RestartableError(
                *Task,
                TEXT("Unable to reserve dedicated server: Can't initialize beacon client to connect to server"));
            return;
        }

        // Force the beacon to use the user ID of the queued player, instead of just
        // the first local player controller in the game.
        Beacon->GetNetConnection()->PlayerId = Task->Request.UserId;
    }
}

bool FMatchmakingAlgorithmStepReserveDedicatedServerForMatch::CanHandleInterrupt(
    const TSharedRef<FMatchmakingEngineTask> &Task,
    FName InterruptType)
{
    if (InterruptType.IsEqual(MMINTERRUPT_HAS_SPECIAL_HANDLING_OF_ABANDONMENT))
    {
        // We have special cleanup to do if we need to abandon.
        return true;
    }

    return false;
}

bool FMatchmakingAlgorithmStepReserveDedicatedServerForMatch::HandleInterrupt(
    const TSharedRef<FMatchmakingEngineTask> &Task,
    FName InterruptType)
{
    if (InterruptType.IsEqual(MMINTERRUPT_ABANDON_CANCEL) || InterruptType.IsEqual(MMINTERRUPT_ABANDON_RESTART))
    {
        // Need to destroy beacon actor, if it still exists.
        auto State = this->GetState(*Task);
        if (State->BeaconActor.IsValid())
        {
            State->BeaconActor->DestroyBeacon();
        }

        // We're not handling the abandonment, tell the event handler it still needs to move to the abandon step.
        return false;
    }

    return false;
}

#endif

REDPOINT_EOS_CODE_GUARD_END()