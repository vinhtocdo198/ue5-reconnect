// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "DedicatedServerReservationTeam.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

#include "DedicatedServerReservation.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

USTRUCT(BlueprintType)
struct MATCHMAKING_API FDedicatedServerReservation
{
    GENERATED_BODY();

public:
    /**
     * A list of the teams in the reservation.
     */
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Matchmaking")
    TArray<FDedicatedServerReservationTeam> Teams;
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()