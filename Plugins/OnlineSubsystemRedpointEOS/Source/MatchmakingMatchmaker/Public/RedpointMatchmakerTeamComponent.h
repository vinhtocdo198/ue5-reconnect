// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "MatchmakerSubsystem.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointNotifyUniqueIdInterface.h"

#include "RedpointMatchmakerTeamComponent.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

/**
 * This component should be attached to player states. It provides access to the team and slot calculated by the
 * matchmaker.
 *
 * Unlike the matchmaker subsystem, if the game map is running without the matchmaker having run (such as during
 * play-in-editor), it automatically allocates each player to their own team. This makes it the preferred way of
 * managing teams and slots going forward.
 */
UCLASS(
    BlueprintType,
    Category = "Redpoint EOS Online Framework|Matchmaking",
    HideCategories = (LOD, Lighting, Transform, Sockets, TextureStreaming),
    Meta = (BlueprintSpawnableComponent))
class MATCHMAKINGMATCHMAKER_API URedpointMatchmakerTeamComponent : public UActorComponent,
                                                                   public IRedpointNotifyUniqueIdInterface
{
    GENERATED_BODY()

private:
    UPROPERTY(Replicated)
    int32 Team;

    UPROPERTY(Replicated)
    int32 Slot;

    FDateTime CreatedAt;

public:
    URedpointMatchmakerTeamComponent(const FObjectInitializer &ObjectInitializer);

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;

private:
    void GetTeamAndSlotForPlayer(
        UMatchmakerSubsystem *Subsystem,
        const FUniqueNetIdRepl &LocalPlayerId,
        const FUniqueNetIdRepl &TargetPlayerId,
        bool &Found,
        int32 &Team,
        int32 &Slot);

    void GetTeamAndSlotForPlayerOnDedicatedServer(
        UMatchmakerSubsystem *Subsystem,
        const FUniqueNetIdRepl &TargetPlayerId,
        bool &Found,
        int32 &Team,
        int32 &Slot);

public:
    virtual void NotifyUniqueNetIdSet_Implementation(APlayerState *PlayerState, FUniqueNetIdRepl const &UniqueId)
        override;

    /**
     * Return the team number that this player is assigned to.
     */
    UFUNCTION(BlueprintPure, Category = "Redpoint EOS Online Framework|Matchmaking")
    int32 GetTeam() const;

    /**
     * Return the slot index within the team that this player is assigned to.
     */
    UFUNCTION(BlueprintPure, Category = "Redpoint EOS Online Framework|Matchmaking")
    int32 GetSlot() const;
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()
