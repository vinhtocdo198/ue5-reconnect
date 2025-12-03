// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Containers/Ticker.h"
#include "DedicatedServerReservationSlot.h"

#include "DedicatedServerReservationTeam.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

USTRUCT(BlueprintType)
struct MATCHMAKING_API FDedicatedServerReservationTeam
{
    GENERATED_BODY();

public:
    /**
     * A list of the slots in this team. Each slot indicates if it's been filled and if so, what user ID is in this
     * slot. The number of slots always matches the desired match layout (e.g. there will always be two entries in a 2v2
     * match). Some slots may have bHasUser = false if the matchmaker was only able to partially fill the match.
     */
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Matchmaking")
    TArray<FDedicatedServerReservationSlot> Slots;
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()