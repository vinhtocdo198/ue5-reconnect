// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "GameFramework/PlayerStart.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointMatchmakerTeamPlayerStart.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

/**
 * An implementation of APlayerStart that contains team and slot information.
 */
UCLASS(BlueprintType, Category = "Redpoint EOS Online Framework|Matchmaking")
class MATCHMAKINGMATCHMAKER_API ARedpointMatchmakerTeamPlayerStart : public APlayerStart
{
    GENERATED_BODY()

public:
    /**
     * The team that this player start is for.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Redpoint EOS Online Framework|Matchmaking")
    int32 Team;

    /**
     * The slot within the team that this player start is for.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Redpoint EOS Online Framework|Matchmaking")
    int32 Slot;
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()
