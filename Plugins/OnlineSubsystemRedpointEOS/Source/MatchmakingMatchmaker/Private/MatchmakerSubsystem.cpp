// Copyright June Rhodes. All Rights Reserved.

#include "MatchmakerSubsystem.h"

#include "DedicatedServerMatchmakingBeaconHost.h"
#include "Engine/LocalPlayer.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerState.h"
#include "OnlineSubsystemUtils.h"
#include "UObject/Object.h"
#if WITH_EDITOR
#include "RedpointEOSCore/Editor/EditorSignalling.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

void UMatchmakerSubsystem::OnNativeMatchmakingResultsReady(
    const FString &TaskId,
    const struct FMatchmakingEngineRequest &Request,
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    FMatchmakingEngineResponse Response)
{
    TSharedPtr<IOnlineIdentity, ESPMode::ThreadSafe> Identity = Request.Identity.Pin();
    if (!Identity.IsValid())
    {
        return;
    }

    // Figure out the controller index for the request user.
    int ControllerIndex = -1;
    for (int i = 0; i < MAX_LOCAL_PLAYERS; i++)
    {
        if (Identity->GetLoginStatus(i) != ELoginStatus::NotLoggedIn)
        {
            TSharedPtr<const FUniqueNetId> LocalUserId = Identity->GetUniquePlayerId(i);
            if (LocalUserId.IsValid() && *LocalUserId == *Request.UserId)
            {
                ControllerIndex = i;
                break;
            }
        }
    }
    if (ControllerIndex == -1)
    {
        return;
    }

    FMatchmakerResult Result;
    for (const auto &Team : Response.Teams)
    {
        FMatchmakerResultTeam TeamBP;
        for (int i = 0; i < Team.Slots.Num(); i++)
        {
            if (Team.Slots[i].Type == EMatchmakingEngineResponseTeamSlotType::User)
            {
                TeamBP.Slots.Add(i, FUniqueNetIdRepl(Team.Slots[i].UserId));
            }
            else
            {
                TeamBP.Slots.Add(i, FUniqueNetIdRepl());
            }
        }
        Result.Teams.Add(TeamBP);
    }

    // Store into the cache.
    this->CachedResponse = Response;
    this->CachedSkillStatPrefix =
        Request.HostConfiguration.IsValid() ? Request.HostConfiguration->SkillStatPrefix : TEXT("");
    this->CachedResults.Add(FUniqueNetIdRepl(Request.UserId), Result);
}

void UMatchmakerSubsystem::OnNativeMatchmakingComplete(
    const FString &TaskId,
    const struct FMatchmakingEngineRequest &Request,
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    FMatchmakingEngineResponse Response)
{
    TSharedPtr<IOnlineIdentity, ESPMode::ThreadSafe> Identity = Request.Identity.Pin();
    if (!Identity.IsValid())
    {
        return;
    }

    // Figure out the controller index for the request user.
    int ControllerIndex = -1;
    for (int i = 0; i < MAX_LOCAL_PLAYERS; i++)
    {
        if (Identity->GetLoginStatus(i) != ELoginStatus::NotLoggedIn)
        {
            TSharedPtr<const FUniqueNetId> LocalUserId = Identity->GetUniquePlayerId(i);
            if (LocalUserId.IsValid() && *LocalUserId == *Request.UserId)
            {
                ControllerIndex = i;
                break;
            }
        }
    }
    if (ControllerIndex == -1)
    {
        return;
    }

    FMatchmakerResult Result;
    for (const auto &Team : Response.Teams)
    {
        FMatchmakerResultTeam TeamBP;
        for (int i = 0; i < Team.Slots.Num(); i++)
        {
            if (Team.Slots[i].Type == EMatchmakingEngineResponseTeamSlotType::User)
            {
                TeamBP.Slots.Add(i, FUniqueNetIdRepl(Team.Slots[i].UserId));
            }
            else
            {
                TeamBP.Slots.Add(i, FUniqueNetIdRepl());
            }
        }
        Result.Teams.Add(TeamBP);
    }

    // Store into the cache.
    this->CachedResponse = Response;
    this->CachedSkillStatPrefix =
        Request.HostConfiguration.IsValid() ? Request.HostConfiguration->SkillStatPrefix : TEXT("");
    this->CachedResults.Add(FUniqueNetIdRepl(Request.UserId), Result);

    // Fire the event.
    this->OnComplete.Broadcast(ControllerIndex, FUniqueNetIdRepl(Request.UserId), Result);
}

#endif

void UMatchmakerSubsystem::GetTeamAndSlotForPlayer(
    APlayerController *LocalPlayerController,
    APlayerState *PlayerState,
    bool &Found,
    int32 &Team,
    int32 &Slot)
{
    Found = false;
    Team = -1;
    Slot = -1;

#if !REDPOINT_EOS_IS_FREE_EDITION
    if (!IsValid(PlayerState))
    {
        UE_LOG(
            LogMatchmakingMatchmaker,
            Error,
            TEXT("The target player state is not valid during the call to GetTeamAndSlotForPlayer."));
        return;
    }

    ULocalPlayer *LocalPlayer = nullptr;
    if (!IsValid(LocalPlayerController))
    {
        if (!IsValid(this->GetWorld()))
        {
            UE_LOG(
                LogMatchmakingMatchmaker,
                Error,
                TEXT("No player controller provided to GetTeamAndSlotForPlayer, and the current world is not "
                     "accessible."));
            return;
        }

        if (this->GetWorld()->GetNetMode() == NM_DedicatedServer)
        {
            UE_LOG(
                LogMatchmakingMatchmaker,
                Warning,
                TEXT("You should use GetTeamAndSlotForPlayerOnServer instead of GetTeamAndSlotForPlayer if your game "
                     "is using dedicated servers!"));
            this->GetTeamAndSlotForPlayerOnServer(PlayerState, Found, Team, Slot);
            return;
        }

        LocalPlayer = this->GetWorld()->GetFirstLocalPlayerFromController();
    }
    else
    {
        LocalPlayer = LocalPlayerController->GetLocalPlayer();
    }

    FUniqueNetIdRepl LocalUserId;
    if (!IsValid(LocalPlayer))
    {
        ENetMode NetMode = LocalPlayerController->GetNetMode();
        if (NetMode == NM_ListenServer || NetMode == NM_Standalone)
        {
            // We hit this scenario during ChoosePlayerStart when the game host is setting up it's local player
            // controller's spawn position. Due to the inconsistent state of the player controller, state, etc. we just
            // go straight to the online subsystem to get the unique net ID for that controller ID.
            // IOnlineIdentityPtr Identity = Online::GetIdentityInterface(this->GetWorld());
            // LocalUserId = Identity->GetUniquePlayerId(LocalPlayerController->index)
            APlayerState *LocalPlayerState = LocalPlayerController->GetPlayerState<APlayerState>();
            if (IsValid(LocalPlayerState))
            {
                LocalUserId = LocalPlayerState->GetUniqueId();
            }
        }
        else
        {
            UE_LOG(
                LogMatchmakingMatchmaker,
                Error,
                TEXT("The player controller passed to GetTeamAndSlotForPlayer must be a local player on the current "
                     "machine, not a networked player."));
            return;
        }
    }
    else
    {
        LocalUserId = LocalPlayer->GetPreferredUniqueNetId();
    }

    if (!LocalUserId.IsValid())
    {
        UE_LOG(
            LogMatchmakingMatchmaker,
            Error,
            TEXT("The provided local player controller didn't have a valid unique net ID, which is required to "
                 "retrieve matchmaking results."));
        return;
    }

    if (!this->CachedResults.Contains(LocalUserId))
    {
        return;
    }

    for (int32 TeamIndex = 0; TeamIndex < this->CachedResults[LocalUserId].Teams.Num(); TeamIndex++)
    {
        for (int32 SlotIndex = 0; SlotIndex < this->CachedResults[LocalUserId].Teams[TeamIndex].Slots.Num();
             SlotIndex++)
        {
            const auto &SlotUserId = this->CachedResults[LocalUserId].Teams[TeamIndex].Slots[SlotIndex];
            if (SlotUserId == PlayerState->GetUniqueId())
            {
                Found = true;
                Team = TeamIndex;
                Slot = SlotIndex;
                return;
            }
        }
    }
#else
    UE_LOG(
        LogMatchmakingMatchmaker,
        Error,
        TEXT("Team-based matchmaking is not available in the Free Edition. Please upgrade to the paid edition on Fab "
             "to use this feature."));
#if WITH_EDITOR
    ::Redpoint::EOS::Core::Editor::FEditorSignalling::OnEditorCustomSignal().Broadcast(
        TEXT("Matchmaking"),
        TEXT("MatchmakingNotAvailableFreeEdition"));
#endif
#endif
}

void UMatchmakerSubsystem::GetTeamAndSlotForPlayerOnServer(
    APlayerState *PlayerState,
    bool &Found,
    int32 &Team,
    int32 &Slot)
{
    Found = false;
    Team = -1;
    Slot = -1;

#if !REDPOINT_EOS_IS_FREE_EDITION
    if (!IsValid(this->GetWorld()))
    {
        UE_LOG(
            LogMatchmakingMatchmaker,
            Error,
            TEXT("The current world is not accessible while calling GetTeamAndSlotForPlayerOnServer."));
        return;
    }

    if (this->GetWorld()->GetNetMode() == NM_DedicatedServer)
    {
        for (TActorIterator<ADedicatedServerMatchmakingBeaconHost> ActorItr(GetWorld()); ActorItr; ++ActorItr)
        {
            ADedicatedServerMatchmakingBeaconHost *BeaconHost = *ActorItr;
            if (IsValid(BeaconHost))
            {
                for (const auto &Member : BeaconHost->ReservationMembers)
                {
                    if (Member.UserId == *PlayerState->GetUniqueId())
                    {
                        Found = true;
                        Team = Member.Team;
                        Slot = Member.Slot;
                        return;
                    }
                }
            }
        }
        return;
    }
    else if (this->GetWorld()->GetNetMode() == NM_ListenServer || this->GetWorld()->GetNetMode() == NM_Standalone)
    {
        this->GetTeamAndSlotForPlayer(this->GetWorld()->GetFirstPlayerController(), PlayerState, Found, Team, Slot);
        return;
    }
    else
    {
        UE_LOG(LogMatchmakingMatchmaker, Error, TEXT("GetTeamAndSlotForPlayerOnServer can not be called on clients."));
        return;
    }
#else
    UE_LOG(
        LogMatchmakingMatchmaker,
        Error,
        TEXT("Team-based matchmaking is not available in the Free Edition. Please upgrade to the paid edition on Fab "
             "to use this feature."));
#if WITH_EDITOR
    ::Redpoint::EOS::Core::Editor::FEditorSignalling::OnEditorCustomSignal().Broadcast(
        TEXT("Matchmaking"),
        TEXT("MatchmakingNotAvailableFreeEdition"));
#endif
#endif
}

REDPOINT_EOS_CODE_GUARD_END()