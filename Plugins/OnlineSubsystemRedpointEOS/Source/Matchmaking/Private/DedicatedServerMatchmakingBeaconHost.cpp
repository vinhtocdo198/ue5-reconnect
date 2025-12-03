// Copyright June Rhodes. All Rights Reserved.

#include "DedicatedServerMatchmakingBeaconHost.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "DedicatedServerMatchmakingBeaconClient.h"
#include "DedicatedServerMatchmakingBeaconHostObject.h"
#include "Engine/NetConnection.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#endif
#include "RedpointMatchmaking/MatchmakingLog.h"
#if WITH_EDITOR
#include "RedpointEOSCore/Editor/EditorSignalling.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

ADedicatedServerMatchmakingBeaconHost::ADedicatedServerMatchmakingBeaconHost()
{
    this->bOnlyRelevantToOwner = true;
    this->bAlwaysRelevant = false;
    this->bNetLoadOnClient = false;
    this->bRelevantForNetworkReplays = false;
    this->bReplicates = false;
}

#if !REDPOINT_EOS_IS_FREE_EDITION

bool ADedicatedServerMatchmakingBeaconHost::CheckForReservationRelease(float DeltaSeconds)
{
    checkf(
        !this->bIsMatchPlaying,
        TEXT("CheckForReservationRelease should not be called if the match is playing; make sure the timer is cleared "
             "from the ticker!"));

    bool bMatchLeaderPresent = false;
    TSet<FString> AllUserIds;
    FString LeaderUserId;
    for (const auto &Member : this->ReservationMembers)
    {
        if (Member.SlotType == 2)
        {
            if (LeaderUserId.IsEmpty())
            {
                LeaderUserId = Member.UserId.ToString();
            }

            AllUserIds.Add(Member.UserId.ToString());
        }
    }
    for (FConstPlayerControllerIterator It = this->GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        if (!It->IsValid())
        {
            continue;
        }

        APlayerController *PC = It->Get();
        UNetConnection *Conn = Cast<UNetConnection>(PC->Player);
        if (IsValid(Conn))
        {
            FString PlayerId = Conn->PlayerId.ToString();
            if (PlayerId == LeaderUserId)
            {
                bMatchLeaderPresent = true;
            }
            AllUserIds.Remove(PlayerId);
        }
    }

    // Determine if the reservation failed based on settings.
    if (!bMatchLeaderPresent || (this->bRequireAllPlayersConnectedForMatchStart && AllUserIds.Num() > 0))
    {
        UE_LOG(
            LogMatchmaking,
            Error,
            TEXT("Reservation request: Failed: Server timed out waiting for all players to join"));

        this->bIsReserved = false;
        this->ReservationMembers.Reset();
        this->RemainingReservationMembers.Reset();
        this->ReservedByUserId = FUniqueNetIdRepl();
        this->ReservationTimeoutHandle.Reset();

        for (FConstPlayerControllerIterator It = this->GetWorld()->GetPlayerControllerIterator(); It; ++It)
        {
            if (!It->IsValid())
            {
                continue;
            }

            APlayerController *PC = It->Get();
            if (Cast<UNetConnection>(PC->Player) != nullptr)
            {
                if (PC->GetPawn() != nullptr)
                {
                    PC->GetPawn()->Destroy();
                }
                PC->ClientWasKicked(NSLOCTEXT(
                    "Matchmaking",
                    "ClientKickedDueToServerReservation",
                    "Server reservation was terminated because not all clients joined in time."));
                PC->Destroy();
            }
            else
            {
                UE_LOG(
                    LogMatchmaking,
                    Warning,
                    TEXT("Unable to kick player controller because it's not a networked player."));
            }
        }

        this->OnReservationStatusChanged.Broadcast(false, FDedicatedServerReservation());
    }
    else
    {
        UE_LOG(
            LogMatchmaking,
            Warning,
            TEXT(
                "Reservation request: Starting: Server is starting match due to time out waiting for players to join"));
        this->StartMatch();
    }

    return false;
}

void ADedicatedServerMatchmakingBeaconHost::StartMatch()
{
    FTSTicker::GetCoreTicker().RemoveTicker(this->ReservationTimeoutHandle);
    this->ReservationTimeoutHandle.Reset();
    this->OnMatchReadyToStart.Broadcast(this->CreateReservationInfoForEvent());
    this->bIsMatchPlaying = true;
}

bool ADedicatedServerMatchmakingBeaconHost::ReserveForMatch(
    const FUniqueNetIdRepl &ClientUserId,
    const TArray<FDedicatedServerMatchmakingBeaconClientMatchMember> &MatchMembers,
    const TArray<int32> &TeamSizes)
{
    if (this->bIsReserved)
    {
        UE_LOG(LogMatchmaking, Error, TEXT("Reservation request: Failed: Server is already reserved"));
        return false;
    }

    this->bIsReserved = true;
    this->ReservationMembers = MatchMembers;
    this->ReservedByUserId = ClientUserId;
    this->ReservationTimeoutHandle = FTSTicker::GetCoreTicker().AddTicker(
        FTickerDelegate::CreateUObject(this, &ADedicatedServerMatchmakingBeaconHost::CheckForReservationRelease),
        this->ReservationTimeoutSeconds);

    this->RemainingReservationMembers.Reset();
    TArray<FString> AuthorizedUsers;
    for (const auto &Member : this->ReservationMembers)
    {
        if (Member.SlotType == 2)
        {
            this->RemainingReservationMembers.Add(Member.UserId.ToString());
            AuthorizedUsers.Add(Member.UserId.ToString());
        }
    }
    this->ReservationTeamSizes = TeamSizes;

    this->OnReservationStatusChanged.Broadcast(true, this->CreateReservationInfoForEvent());

    UE_LOG(
        LogMatchmaking,
        Verbose,
        TEXT("Reservation request: Notice: Server is now reserved for %s with %d members:"),
        *ClientUserId.ToString(),
        AuthorizedUsers.Num());
    for (const auto &AuthorizedUser : AuthorizedUsers)
    {
        UE_LOG(LogMatchmaking, Verbose, TEXT("  Authorization allows user: %s"), *AuthorizedUser);
    }
    return true;
}

void ADedicatedServerMatchmakingBeaconHost::OnPreLogin(
    AGameModeBase *GameMode,
    const FUniqueNetIdRepl &NewPlayer,
    FString &ErrorMessage)
{
    if (!this->bIsReserved)
    {
        ErrorMessage = TEXT("Dedicated server is not reserved");
        return;
    }

    if (this->bIsMatchPlaying && !this->bAllowPlayerReconnections)
    {
        ErrorMessage = TEXT("Dedicated server does not permit reconnections");
        return;
    }

    bool bFoundMember = false;
    TArray<FString> AuthorizedUsers;
    for (const auto &Member : this->ReservationMembers)
    {
        if (Member.SlotType == 2)
        {
            AuthorizedUsers.Add(Member.UserId.ToString());
            if (Member.UserId.ToString() == NewPlayer.ToString())
            {
                // This user is permitted to join.
                this->RemainingReservationMembers.Remove(NewPlayer.ToString());
                UE_LOG(
                    LogMatchmaking,
                    Verbose,
                    TEXT("Reservation request: Connection: Found user ID %s in permitted list of users to join this "
                         "dedicated server"),
                    *NewPlayer.ToString());
                bFoundMember = true;
            }
        }
    }

    if (bFoundMember)
    {
        if (this->RemainingReservationMembers.Num() == 0 && !this->bIsMatchPlaying)
        {
            // Start the game, we have no other players to wait for.
            UE_LOG(
                LogMatchmaking,
                Verbose,
                TEXT("Reservation request: Starting: Starting the match since all players have now joined"),
                *NewPlayer.ToString());
            this->StartMatch();
        }

        // This connection is permitted.
        return;
    }

    if (AuthorizedUsers.Num() == 0)
    {
        UE_LOG(
            LogMatchmaking,
            Warning,
            TEXT("Rejecting connection from %s because there aren't any authorized users"),
            *NewPlayer.ToString());
    }
    else
    {
        UE_LOG(
            LogMatchmaking,
            Warning,
            TEXT("Rejecting connection from %s because they aren't in the list of authorized users:"),
            *NewPlayer.ToString());
        for (const auto &AuthorizedUser : AuthorizedUsers)
        {
            UE_LOG(LogMatchmaking, Warning, TEXT("  Authorization allows user: %s"), *AuthorizedUser);
        }
    }

    ErrorMessage = TEXT("Dedicated server does not permit connection from this user (not in reservation)");
    return;
}

void ADedicatedServerMatchmakingBeaconHost::OnPostLogin(AGameModeBase *GameMode, APlayerController *NewPlayer)
{
    if (IsValid(NewPlayer->GetNetConnection()))
    {
        FTrackedPlayerController Info;
        Info.PC = NewPlayer;
        Info.UserId = NewPlayer->GetNetConnection()->PlayerId;
        this->TrackedConnectedPlayers.Add(Info);
    }
}

void ADedicatedServerMatchmakingBeaconHost::OnLogout(AGameModeBase *GameMode, AController *Controller)
{
    if (!IsValid(Controller))
    {
        return;
    }

    if (this->bIsMatchPlaying)
    {
        // We don't care about tracking players after the match starts.
        return;
    }

    for (int i = this->TrackedConnectedPlayers.Num() - 1; i >= 0; i--)
    {
        if (this->TrackedConnectedPlayers[i].PC == Controller)
        {
            bool bIsInReservation = false;
            for (const auto &Member : this->ReservationMembers)
            {
                if (Member.SlotType == 2)
                {
                    if (Member.UserId.ToString() == this->TrackedConnectedPlayers[i].UserId.ToString())
                    {
                        bIsInReservation = true;
                        break;
                    }
                }
            }

            if (bIsInReservation)
            {
                this->RemainingReservationMembers.Add(this->TrackedConnectedPlayers[i].UserId.ToString());
                UE_LOG(
                    LogMatchmaking,
                    Warning,
                    TEXT("Reservation request: Connection: Disconnecting user ID %s is being added to the list of "
                         "pending players again"),
                    *this->TrackedConnectedPlayers[i].UserId.ToString());
            }
            this->TrackedConnectedPlayers.RemoveAt(i);
        }
    }
}

FDedicatedServerReservation ADedicatedServerMatchmakingBeaconHost::CreateReservationInfoForEvent() const
{
    FDedicatedServerReservation ReservationInfo;
    for (const auto &TeamSize : this->ReservationTeamSizes)
    {
        FDedicatedServerReservationTeam Team;
        for (int s = 0; s < TeamSize; s++)
        {
            Team.Slots.Add(FDedicatedServerReservationSlot());
        }
        ReservationInfo.Teams.Add(Team);
    }
    for (const auto &Member : this->ReservationMembers)
    {
        if (Member.Team >= 0 && Member.Team < ReservationInfo.Teams.Num())
        {
            if (Member.Slot >= 0 && Member.Slot < ReservationInfo.Teams[Member.Team].Slots.Num())
            {
                if (Member.SlotType == 2)
                {
                    ReservationInfo.Teams[Member.Team].Slots[Member.Slot].UserId = Member.UserId;
                    ReservationInfo.Teams[Member.Team].Slots[Member.Slot].bHasUser = true;
                }
                else
                {
                    ReservationInfo.Teams[Member.Team].Slots[Member.Slot].UserId = FUniqueNetIdRepl();
                    ReservationInfo.Teams[Member.Team].Slots[Member.Slot].bHasUser = false;
                }
            }
        }
    }
    return ReservationInfo;
}

template <typename ClassType, typename ClassType::MemberType Member> struct TBeaconHostAccessShim
{
    friend typename ClassType::MemberType GetBeaconHostMember(ClassType)
    {
        return Member;
    }
};
struct FBeaconHostAuthRequiredTag
{
    typedef bool AOnlineBeaconHost::*MemberType;
    friend MemberType GetBeaconHostMember(FBeaconHostAuthRequiredTag);
};
template struct TBeaconHostAccessShim<FBeaconHostAuthRequiredTag, &AOnlineBeaconHost::bAuthRequired>;

void ADedicatedServerMatchmakingBeaconHost::BeginPlay()
{
    if (!IsValid(GetWorld()))
    {
        UE_LOG(
            LogMatchmaking,
            Error,
            TEXT("ADedicatedServerMatchmakingBeaconHostActor is destroying itself because there is no valid current "
                 "world."));
        this->Destroy();
        return;
    }

    if (this->GetWorld()->GetNetMode() != NM_DedicatedServer)
    {
        UE_LOG(
            LogMatchmaking,
            Verbose,
            TEXT("ADedicatedServerMatchmakingBeaconHostActor is destroying itself because this game is not running as "
                 "a dedicated server (this->GetWorld()->GetNetMode() != NM_DedicatedServer)."));
        this->Destroy();
        return;
    }

    this->bIsReserved = false;
    this->bIsMatchPlaying = false;
    this->ReservedByUserId = FUniqueNetIdRepl();
    this->ReservationTimeoutHandle.Reset();
    this->ReservationMembers.Reset();
    this->RemainingReservationMembers.Reset();

    FGameModeEvents::GameModePreLoginEvent.AddUObject(this, &ADedicatedServerMatchmakingBeaconHost::OnPreLogin);
    FGameModeEvents::GameModePostLoginEvent.AddUObject(this, &ADedicatedServerMatchmakingBeaconHost::OnPostLogin);
    FGameModeEvents::GameModeLogoutEvent.AddUObject(this, &ADedicatedServerMatchmakingBeaconHost::OnLogout);

    this->OnReservationStatusChanged.Broadcast(false, FDedicatedServerReservation());

    AOnlineBeaconHost *BeaconHost = GetWorld()->SpawnActor<AOnlineBeaconHost>(AOnlineBeaconHost::StaticClass());
    BeaconHost->ListenPort = this->BeaconPort;
    {
        // Force beacon to have authentication turned off, even if it's enabled in the configs.
        bool &bAuthRequired = BeaconHost->*GetBeaconHostMember(FBeaconHostAuthRequiredTag());
        bAuthRequired = false;
    }
    if (BeaconHost && BeaconHost->InitHost())
    {
        ADedicatedServerMatchmakingBeaconHostObject *BeaconHostObject =
            GetWorld()->SpawnActor<ADedicatedServerMatchmakingBeaconHostObject>();
        if (BeaconHostObject)
        {
            BeaconHostObject->Init(this);
            BeaconHost->RegisterHost(BeaconHostObject);
            Beacon = BeaconHost;
            BeaconHost->PauseBeaconRequests(false);
        }
        else
        {
            UE_LOG(
                LogMatchmaking,
                Error,
                TEXT("ADedicatedServerMatchmakingBeaconHostActor failed to initialize the beacon object for "
                     "matchmaking. Check the above logs for issues."));
            this->Destroy();
            return;
        }
    }
    else
    {
        UE_LOG(
            LogMatchmaking,
            Error,
            TEXT("ADedicatedServerMatchmakingBeaconHostActor failed to initialize the beacon for matchmaking. Check "
                 "the above logs for issues."));
        this->Destroy();
        return;
    }

    UE_LOG(
        LogMatchmaking,
        Verbose,
        TEXT("ADedicatedServerMatchmakingBeaconHostActor is now waiting for a client to reserve this server."));

    Super::BeginPlay();
}

void ADedicatedServerMatchmakingBeaconHost::BeginDestroy()
{
    if (Beacon)
    {
        Beacon->DestroyBeacon();
        Beacon = nullptr;
    }

    UE_LOG(LogMatchmaking, Verbose, TEXT("ADedicatedServerMatchmakingBeaconHostActor has been destroyed."));

    Super::BeginDestroy();
}

#endif

void ADedicatedServerMatchmakingBeaconHost::MatchEnded()
{
#if !REDPOINT_EOS_IS_FREE_EDITION
    if (!this->bIsMatchPlaying)
    {
        UE_LOG(
            LogMatchmaking,
            Error,
            TEXT("Reservation request: Error: Ignoring EndMatch call, we are not currently playing a match"));
        return;
    }

    UE_LOG(
        LogMatchmaking,
        Verbose,
        TEXT("Reservation request: Ended: Match ended for %s, server is now unreserved and ready for new clients to "
             "connect"),
        *this->ReservedByUserId.ToString());

    this->bIsMatchPlaying = false;
    this->bIsReserved = false;
    this->ReservationMembers.Reset();
    this->RemainingReservationMembers.Reset();
    this->ReservedByUserId = FUniqueNetIdRepl();
    this->ReservationTimeoutHandle.Reset();
#else
    UE_LOG(
        LogMatchmaking,
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

bool ADedicatedServerMatchmakingBeaconHost::IsReserved() const
{
#if !REDPOINT_EOS_IS_FREE_EDITION
    return this->bIsReserved;
#else
    UE_LOG(
        LogMatchmaking,
        Error,
        TEXT("Team-based matchmaking is not available in the Free Edition. Please upgrade to the paid edition on Fab "
             "to use this feature."));
#if WITH_EDITOR
    ::Redpoint::EOS::Core::Editor::FEditorSignalling::OnEditorCustomSignal().Broadcast(
        TEXT("Matchmaking"),
        TEXT("MatchmakingNotAvailableFreeEdition"));
#endif
    return false;
#endif
}

bool ADedicatedServerMatchmakingBeaconHost::IsMatchPlaying() const
{
#if !REDPOINT_EOS_IS_FREE_EDITION
    return this->bIsMatchPlaying;
#else
    UE_LOG(
        LogMatchmaking,
        Error,
        TEXT("Team-based matchmaking is not available in the Free Edition. Please upgrade to the paid edition on Fab "
             "to use this feature."));
#if WITH_EDITOR
    ::Redpoint::EOS::Core::Editor::FEditorSignalling::OnEditorCustomSignal().Broadcast(
        TEXT("Matchmaking"),
        TEXT("MatchmakingNotAvailableFreeEdition"));
#endif
    return false;
#endif
}

TSet<FString> ADedicatedServerMatchmakingBeaconHost::GetRemainingReservationMembers() const
{
#if !REDPOINT_EOS_IS_FREE_EDITION
    return this->RemainingReservationMembers;
#else
    UE_LOG(
        LogMatchmaking,
        Error,
        TEXT("Team-based matchmaking is not available in the Free Edition. Please upgrade to the paid edition on Fab "
             "to use this feature."));
#if WITH_EDITOR
    ::Redpoint::EOS::Core::Editor::FEditorSignalling::OnEditorCustomSignal().Broadcast(
        TEXT("Matchmaking"),
        TEXT("MatchmakingNotAvailableFreeEdition"));
#endif
    return TSet<FString>();
#endif
}

void ADedicatedServerMatchmakingBeaconHost::GetCurrentReservation(
    bool &bOutIsReserved,
    FDedicatedServerReservation &OutReservation) const
{
#if !REDPOINT_EOS_IS_FREE_EDITION
    bOutIsReserved = this->bIsReserved;
    if (this->bIsReserved)
    {
        OutReservation = this->CreateReservationInfoForEvent();
    }
#else
    UE_LOG(
        LogMatchmaking,
        Error,
        TEXT("Team-based matchmaking is not available in the Free Edition. Please upgrade to the paid edition on Fab "
             "to use this feature."));
#if WITH_EDITOR
    ::Redpoint::EOS::Core::Editor::FEditorSignalling::OnEditorCustomSignal().Broadcast(
        TEXT("Matchmaking"),
        TEXT("MatchmakingNotAvailableFreeEdition"));
#endif
    bOutIsReserved = false;
#endif
}

REDPOINT_EOS_CODE_GUARD_END()