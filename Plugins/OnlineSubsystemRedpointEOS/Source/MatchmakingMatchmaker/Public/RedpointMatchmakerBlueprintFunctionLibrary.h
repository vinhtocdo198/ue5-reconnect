// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointMatchmakerBlueprintFunctionLibrary.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

UCLASS()
class MATCHMAKINGMATCHMAKER_API URedpointMatchmakerBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /**
     * Given a player state with an URedpointMatchmakerTeamComponent, locates the ARedpointMatchmakerTeamPlayerStart
     * actor in the current map and returns it.
     */
    UFUNCTION(
        BlueprintCallable,
        BlueprintAuthorityOnly,
        Category = "Redpoint EOS Online Framework|Matchmaker",
        meta = (DisplayName = "Find Player Start for Player State with Matchmaker Team Component"))
    static APlayerStart *FindMatchmakerTeamPlayerStart(APlayerState *PlayerState);

    /**
     * Returns true if all of the players expected in the match, based on matchmaking results, have connected to the
     * current server. This should be called in the "ReadyToStartMatch" event of AGameMode, and used to start the game
     * when everyone is connected.
     */
    UFUNCTION(
        BlueprintCallable,
        BlueprintAuthorityOnly,
        Category = "Redpoint EOS Online Framework|Matchmaker",
        meta = (DisplayName = "Are All Expected Players Connected from Matchmaker?"))
    static bool AreAllExpectedPlayersConnectedFromMatchmaker(UGameInstance *GameInstance);
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()