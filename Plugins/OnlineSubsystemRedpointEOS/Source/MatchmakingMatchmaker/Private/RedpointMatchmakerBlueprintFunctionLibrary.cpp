// Copyright June Rhodes. All Rights Reserved.

#include "RedpointMatchmakerBlueprintFunctionLibrary.h"

#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/NetConnection.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerState.h"
#include "LogMatchmakingMatchmaker.h"
#include "MatchmakerSubsystem.h"
#include "RedpointMatchmakerTeamComponent.h"
#include "RedpointMatchmakerTeamPlayerStart.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

APlayerStart *URedpointMatchmakerBlueprintFunctionLibrary::FindMatchmakerTeamPlayerStart(APlayerState *PlayerState)
{
    if (!PlayerState)
    {
        UE_LOG(
            LogMatchmakingMatchmaker,
            Error,
            TEXT("PlayerState passed into FindMatchmakerTeamPlayerStart is not valid!"));
        return nullptr;
    }

    URedpointMatchmakerTeamComponent *Component = PlayerState->GetComponentByClass<URedpointMatchmakerTeamComponent>();
    if (!Component)
    {
        UE_LOG(
            LogMatchmakingMatchmaker,
            Error,
            TEXT("PlayerState passed into FindMatchmakerTeamPlayerStart does not have "
                 "URedpointMatchmakerTeamComponent!"));
        return nullptr;
    }

    int32 Team = Component->GetTeam();
    int32 Slot = Component->GetSlot();
    for (TActorIterator<ARedpointMatchmakerTeamPlayerStart> Itr(PlayerState->GetWorld()); Itr; ++Itr)
    {
        if (Itr->Team == Team && Itr->Slot == Slot)
        {
            UE_LOG(
                LogMatchmakingMatchmaker,
                Verbose,
                TEXT("Incoming player has team %d and slot %d. Selected player start '%s' as starting location."),
                Team,
                Slot,
                *AActor::GetDebugName(*Itr));
            return *Itr;
        }
    }

    UE_LOG(
        LogMatchmakingMatchmaker,
        Error,
        TEXT("Unable to locate ARedpointMatchmakerTeamPlayerStart with team %d and slot %d for incoming player!"),
        Team,
        Slot);
    return nullptr;
}

bool URedpointMatchmakerBlueprintFunctionLibrary::AreAllExpectedPlayersConnectedFromMatchmaker(
    UGameInstance *GameInstance)
{
    if (!IsValid(GameInstance))
    {
        return false;
    }

    UMatchmakerSubsystem *MatchmakerSubsystem = GameInstance->GetSubsystem<UMatchmakerSubsystem>();
    if (!IsValid(MatchmakerSubsystem))
    {
        return false;
    }

    UWorld *CurrentWorld = GameInstance->GetWorld();
    if (!IsValid(CurrentWorld))
    {
        return false;
    }

#if !REDPOINT_EOS_IS_FREE_EDITION
    // Get the user IDs of all connected players.
    TSet<FUniqueNetIdRepl> PlayersPresent;
    for (FConstPlayerControllerIterator It = CurrentWorld->GetPlayerControllerIterator(); It; ++It)
    {
        if (It->IsValid())
        {
            if ((*It)->IsLocalPlayerController())
            {
                ULocalPlayer *LocalPlayer = (*It)->GetLocalPlayer();
                if (IsValid(LocalPlayer))
                {
                    PlayersPresent.Add(LocalPlayer->GetPreferredUniqueNetId());
                    continue;
                }
            }

            UNetConnection *RemoteNetConnection = Cast<UNetConnection>((*It)->Player);
            if (IsValid(RemoteNetConnection))
            {
                PlayersPresent.Add(RemoteNetConnection->PlayerId);
                continue;
            }
        }
    }

    // Check if any are missing.
    for (const auto &TeamKV : MatchmakerSubsystem->CachedResponse.Teams)
    {
        for (const auto &SlotKV : TeamKV.Slots)
        {
            if (SlotKV.Type == EMatchmakingEngineResponseTeamSlotType::User)
            {
                if (!PlayersPresent.Contains(FUniqueNetIdRepl(SlotKV.UserId)))
                {
                    return false;
                }
            }
        }
    }

#if WITH_EDITOR
    // If we are running in the editor, and we don't have a cached response (i.e. we started the game immediately in the
    // multiplayer without going into matchmaking), we need to wait until all play-in-editor clients have started up and
    // signed in before the match can start.
    int ExpectedPlayerControllers = 0;
    UWorld *HostWorld = nullptr;
    for (const auto &WorldContext : GEngine->GetWorldContexts())
    {
        if (WorldContext.WorldType == EWorldType::PIE)
        {
            if (WorldContext.bWaitingOnOnlineSubsystem)
            {
                // Still waiting on this PIE instance to be ready.
                return false;
            }

            if (!WorldContext.RunAsDedicated)
            {
                ExpectedPlayerControllers++;
            }

            UWorld *ThisWorld = WorldContext.World();
            if (IsValid(ThisWorld) &&
                (ThisWorld->GetNetMode() == NM_ListenServer || ThisWorld->GetNetMode() == NM_DedicatedServer))
            {
                HostWorld = ThisWorld;
            }
        }
    }

    // Check to make sure we have all the player controllers connected.
    if (IsValid(HostWorld))
    {
        if (HostWorld->GetNumPlayerControllers() < ExpectedPlayerControllers)
        {
            return false;
        }
    }
#endif

    return true;
#else
    // Free Edition never has team-based matchmaking state, so always return true in this case.
    return true;
#endif
}

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()