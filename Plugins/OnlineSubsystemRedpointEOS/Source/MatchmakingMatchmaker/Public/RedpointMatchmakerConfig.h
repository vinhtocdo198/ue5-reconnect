// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointMatchmakerConfig.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

/**
 * Global settings for team-based matchmaking in the Redpoint EOS Online Framework plugin.
 */
UCLASS(Config = Engine, DefaultConfig)
class MATCHMAKINGMATCHMAKER_API URedpointMatchmakerConfig : public UObject
{
    GENERATED_BODY()

public:
    URedpointMatchmakerConfig();

    /**
     * The default team layout to when a game map is started without matchmaking. This is used by the
     * URedpointMatchmakerTeamComponent to automatically assign teams and slots to players when matchmaking hasn't
     * happened, which is useful for testing game maps in the editor.
     */
    UPROPERTY(EditAnywhere, Config, Category = "Development Scenarios")
    FString TeamLayoutWithoutMatchmaker;
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()