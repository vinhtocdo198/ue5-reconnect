// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRecentPlayersWorldSubsystem.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSCore/Id/UniqueNetIdEOS.h"
#include "RedpointEOSCore/Utils/WorldResolution.h"
#include "RedpointEOSFriends/FriendSystem.h"
#include "RedpointEOSRecentPlayers/Logging.h"
#endif

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

void URecentPlayersWorldSubsystem::OnActorSpawned(AActor *NewActor)
{
    APlayerState *NewPlayerState = Cast<APlayerState>(NewActor);
    if (IsValid(NewPlayerState))
    {
        FUniqueNetIdRepl PlayerStateId = NewPlayerState->GetUniqueId();
        if (PlayerStateId.IsValid())
        {
            this->DiscoveredPlayerId(PlayerStateId);
        }
        else
        {
            // Delay until the player state has a unique net ID set.
            FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(
                this,
                &URecentPlayersWorldSubsystem::OnDelayedPlayerStartEvaluation,
                TSoftObjectPtr<APlayerState>(NewPlayerState)));
        }
    }
}

bool URecentPlayersWorldSubsystem::OnDelayedPlayerStartEvaluation(
    float DeltaSeconds,
    TSoftObjectPtr<APlayerState> InPlayerState)
{
    if (!InPlayerState.IsValid())
    {
        // Player state no longer valid.
        return false;
    }

    FUniqueNetIdRepl PlayerStateId = InPlayerState->GetUniqueId();
    if (PlayerStateId.IsValid())
    {
        this->DiscoveredPlayerId(PlayerStateId);
        return false;
    }

    // Need to try again later.
    return true;
}

void URecentPlayersWorldSubsystem::DiscoveredPlayerId(const FUniqueNetIdRepl &InPlayerId)
{
    using namespace ::Redpoint::EOS::Core::Id;
    using namespace ::Redpoint::EOS::Core::Utils;
    using namespace ::Redpoint::EOS::Friends;
    using namespace ::Redpoint::EOS::RecentPlayers;

    auto PlatformHandle = FWorldResolution::TryGetPlatformHandle(this->GetWorld());
    if (!PlatformHandle.IsValid())
    {
        UE_LOG(
            LogRedpointEOSRecentPlayers,
            Error,
            TEXT("No online services are present, so recent players can not be tracked."));
        return;
    }

    auto FriendSystem = PlatformHandle->GetSystem<IFriendSystem>();
    FriendSystem->RecordRecentPlayer(InPlayerId.IsV1() ? GetAccountId(*InPlayerId.GetV1()) : InPlayerId.GetV2());
}

void URecentPlayersWorldSubsystem::Initialize(FSubsystemCollectionBase &Collection)
{
    this->GetWorld()->AddOnActorSpawnedHandler(
        FOnActorSpawned::FDelegate::CreateUObject(this, &URecentPlayersWorldSubsystem::OnActorSpawned));
}

#endif