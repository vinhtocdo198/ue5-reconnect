// Copyright June Rhodes. All Rights Reserved.

#include "RedpointMatchmakerTeamComponent.h"

#include "DedicatedServerMatchmakingBeaconHost.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "EngineUtils.h"
#include "GameFramework/GameState.h"
#include "MatchmakerSubsystem.h"
#include "Net/UnrealNetwork.h"
#include "RedpointMatchmakerConfig.h"
#include "RedpointMatchmaking/Utility/ParseTeamCapacities.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

URedpointMatchmakerTeamComponent::URedpointMatchmakerTeamComponent(const FObjectInitializer &ObjectInitializer)
    : UActorComponent(ObjectInitializer)
    , Team(0)
    , Slot(0)
{
    this->bWantsInitializeComponent = true;
    this->SetIsReplicatedByDefault(true);
    this->CreatedAt = FDateTime::UtcNow();
}

void URedpointMatchmakerTeamComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(URedpointMatchmakerTeamComponent, Team);
    DOREPLIFETIME(URedpointMatchmakerTeamComponent, Slot);
}

void URedpointMatchmakerTeamComponent::GetTeamAndSlotForPlayer(
    UMatchmakerSubsystem *Subsystem,
    const FUniqueNetIdRepl &LocalPlayerId,
    const FUniqueNetIdRepl &TargetPlayerId,
    bool &OutFound,
    int32 &OutTeam,
    int32 &OutSlot)
{
    OutFound = false;
    OutTeam = -1;
    OutSlot = -1;

#if !REDPOINT_EOS_IS_FREE_EDITION
    if (!Subsystem->CachedResults.Contains(LocalPlayerId))
    {
        return;
    }

    for (int32 TeamIndex = 0; TeamIndex < Subsystem->CachedResults[LocalPlayerId].Teams.Num(); TeamIndex++)
    {
        for (int32 SlotIndex = 0; SlotIndex < Subsystem->CachedResults[LocalPlayerId].Teams[TeamIndex].Slots.Num();
             SlotIndex++)
        {
            const auto &SlotUserId = Subsystem->CachedResults[LocalPlayerId].Teams[TeamIndex].Slots[SlotIndex];
            if (SlotUserId == TargetPlayerId)
            {
                OutFound = true;
                OutTeam = TeamIndex;
                OutSlot = SlotIndex;
                return;
            }
        }
    }
#endif
}

void URedpointMatchmakerTeamComponent::GetTeamAndSlotForPlayerOnDedicatedServer(
    UMatchmakerSubsystem *Subsystem,
    const FUniqueNetIdRepl &TargetPlayerId,
    bool &OutFound,
    int32 &OutTeam,
    int32 &OutSlot)
{
    OutFound = false;
    OutTeam = -1;
    OutSlot = -1;

#if !REDPOINT_EOS_IS_FREE_EDITION
    if (!IsValid(this->GetWorld()))
    {
        UE_LOG(
            LogMatchmakingMatchmaker,
            Error,
            TEXT("The current world is not accessible while calling GetTeamAndSlotForPlayerOnServer."));
        return;
    }

    for (TActorIterator<ADedicatedServerMatchmakingBeaconHost> ActorItr(this->GetWorld()); ActorItr; ++ActorItr)
    {
        ADedicatedServerMatchmakingBeaconHost *BeaconHost = *ActorItr;
        if (IsValid(BeaconHost))
        {
            for (const auto &Member : BeaconHost->ReservationMembers)
            {
                if (Member.UserId == TargetPlayerId)
                {
                    OutFound = true;
                    OutTeam = Member.Team;
                    OutSlot = Member.Slot;
                    return;
                }
            }
        }
    }
#endif
}

void URedpointMatchmakerTeamComponent::NotifyUniqueNetIdSet_Implementation(
    APlayerState *PlayerState,
    FUniqueNetIdRepl const &UniqueId)
{
    using namespace ::Redpoint::Matchmaking::Utility;

    AActor *Actor = this->GetOwner();

    APlayerState *PS = Cast<APlayerState>(Actor);
    if (!PS)
    {
        UE_LOG(
            LogMatchmakingMatchmaker,
            Error,
            TEXT("URedpointMatchmakerTeamComponent can only be attached to player state actors!"));
        return;
    }

    UGameInstance *GameInstance = Actor->GetGameInstance();
    if (!GameInstance)
    {
        UE_LOG(
            LogMatchmakingMatchmaker,
            Warning,
            TEXT("No game instance available when URedpointMatchmakerTeamComponent was initializing!"));
        return;
    }

    UMatchmakerSubsystem *Subsystem = GameInstance->GetSubsystem<UMatchmakerSubsystem>();
    checkf(Subsystem != nullptr, TEXT("Expected UMatchmakerSubsystem to exist!"));

    bool bFound = false;
#if !REDPOINT_EOS_IS_FREE_EDITION
    if (PS->GetUniqueId().IsValid())
    {
        if (this->GetWorld()->GetNetMode() != NM_DedicatedServer)
        {
            auto *LocalPlayer = GameInstance->GetLocalPlayerByIndex(0);
            if (LocalPlayer)
            {
                this->GetTeamAndSlotForPlayer(
                    Subsystem,
                    LocalPlayer->GetCachedUniqueNetId(),
                    PS->GetUniqueId(),
                    bFound,
                    this->Team,
                    this->Slot);
            }
        }
        else
        {
            this->GetTeamAndSlotForPlayerOnDedicatedServer(
                Subsystem,
                PS->GetUniqueId(),
                bFound,
                this->Team,
                this->Slot);
        }
    }
    else
    {
        UE_LOG(
            LogMatchmakingMatchmaker,
            Error,
            TEXT("Player state does not have a unique net ID associated with it, so we can not locate matchmaking "
                 "results for this player."));
    }
#endif

    if (bFound)
    {
        // The team and slot was determined by the matchmaker and will be the same on client and server.
    }
    else if (this->GetOwnerRole() == ENetRole::ROLE_Authority)
    {
        // The server makes this determination and allocates players onto teams. This allows teams and slots to be
        // available when testing in play-in-editor.
        TArray<int32> TeamCapacities =
            ParseTeamCapacities(GetDefault<URedpointMatchmakerConfig>()->TeamLayoutWithoutMatchmaker)
                .Get(TArray<int32>{4, 4});
        this->Team = 0;
        this->Slot = 0;

        // @note: This jank is necessary so we can have a stable order of player states on the server, ordered by the
        // CreatedAt field which is set in the constructor.
        TArray<APlayerState *> AllPlayerStates;
        for (TActorIterator<APlayerState> Itr(this->GetWorld()); Itr; ++Itr)
        {
            AllPlayerStates.Add(*Itr);
        }
        AllPlayerStates.StableSort([](const APlayerState &A, const APlayerState &B) {
            URedpointMatchmakerTeamComponent *AC = A.GetComponentByClass<URedpointMatchmakerTeamComponent>();
            URedpointMatchmakerTeamComponent *BC = B.GetComponentByClass<URedpointMatchmakerTeamComponent>();
            if (AC && BC)
            {
                return AC->CreatedAt < BC->CreatedAt;
            }
            else
            {
                return AC < BC;
            }
        });

        UE_LOG(
            LogMatchmakingMatchmaker,
            Verbose,
            TEXT("There are %d player states present when automatically assigning teams and slots no matchmaking state "
                 "was available."),
            AllPlayerStates.Num());
        for (int32 i = 0; i < AllPlayerStates.Num(); i++)
        {
            if (AllPlayerStates[i] == this->GetOwner())
            {
                UE_LOG(
                    LogMatchmakingMatchmaker,
                    Verbose,
                    TEXT("Automatically assigned player %d to team %d and slot %d because no matchmaking state is "
                         "available."),
                    i,
                    this->Team,
                    this->Slot);
                break;
            }

            this->Slot++;
            auto TeamMax = this->Team < TeamCapacities.Num() ? TeamCapacities[this->Team] : 4;
            if (this->Slot >= TeamMax)
            {
                this->Slot = 0;
                this->Team++;
            }
        }
    }
    else
    {
        // The client waits for the server to replicate the team and slot down to it.
    }
}

int32 URedpointMatchmakerTeamComponent::GetTeam() const
{
    return this->Team;
}

int32 URedpointMatchmakerTeamComponent::GetSlot() const
{
    return this->Slot;
}

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()
