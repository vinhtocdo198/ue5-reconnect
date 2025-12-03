// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "GameFramework/OnlineReplStructs.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Matchmaker.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointMatchmaking/MatchmakingEngine.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "MatchmakerSubsystem.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FMatchmakerComplete,
    int32,
    ControllerIndex,
    const FUniqueNetIdRepl &,
    UserId,
    const FMatchmakerResult &,
    Results);

/**
 * Provides access to the matchmaker's results and an event that fires once the match is setup and ready to play.
 */
UCLASS()
class UMatchmakerSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

    friend class UMatchmakerSubmitMatchResults;

public:
#if !REDPOINT_EOS_IS_FREE_EDITION
    void OnNativeMatchmakingResultsReady(
        const FString &TaskId,
        const struct FMatchmakingEngineRequest &Request,
        FMatchmakingEngineResponse Response);
    void OnNativeMatchmakingComplete(
        const FString &TaskId,
        const struct FMatchmakingEngineRequest &Request,
        FMatchmakingEngineResponse Response);

    TMap<FUniqueNetIdRepl, FMatchmakerResult> CachedResults;
    FString CachedSkillStatPrefix;
    FMatchmakingEngineResponse CachedResponse;
#endif

    /**
     * Returns the team and slot for the target player.
     *
     * This blueprint node works on both clients and listen servers. On a client you can use any player state as the
     * target player (including the player states of other networked players).
     *
     * If this is called on a dedicated server, it falls back to GetTeamAndSlotForPlayerOnServer.
     *
     * The player controller should represent the local player that queued into matchmaking, since you can technically
     * have local players queue differently. However, for almost all games it's suitable to provide the result of "Get
     * Player Controller 0" as the local player controller.
     *
     * If you don't pass a local player controller, it defaults to player controller 0.
     */
    UFUNCTION(BlueprintPure, Category = "Matchmaking")
    void GetTeamAndSlotForPlayer(
        APlayerController *LocalPlayerController,
        APlayerState *TargetPlayerState,
        bool &Found,
        int32 &Team,
        int32 &Slot);

    /**
     * Returns the team and slot for the target player on the server.
     *
     * This blueprint node only works on listen servers and dedicated servers. Unlike GetTeamAndSlotForPlayer, this
     * doesn't require a local player controller since dedicated servers don't have one.
     *
     * On listen servers, this is the same as calling GetTeamAndSlotForPlayer with local player controller 0. On
     * dedicated servers, it uses the matchmaking result data communicated to the server by the matchmaking host via
     * beacon RPCs (prior to anyone joining).
     */
    UFUNCTION(BlueprintPure, Category = "Matchmaking")
    void GetTeamAndSlotForPlayerOnServer(APlayerState *TargetPlayerState, bool &Found, int32 &Team, int32 &Slot);

    /**
     * This event is fired when the matchmaker is complete and the map has been joined. This event is located on the
     * subsystem because the AMatchmaker actor will have been released when the listen server was started or the
     * remote map was joined.
     *
     * This only fires for matches that were started from an AMatchmaker actor; it does not fire if you are using the
     * lower-level C++ API.
     */
    UPROPERTY(BlueprintAssignable, EditAnywhere, Category = "Matchmaking")
    FMatchmakerComplete OnComplete;
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()