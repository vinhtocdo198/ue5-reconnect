// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "GameFramework/OnlineReplStructs.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

#include "DedicatedServerReservationSlot.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

USTRUCT(BlueprintType)
struct MATCHMAKING_API FDedicatedServerReservationSlot
{
    GENERATED_BODY();

public:
    /**
     * A slot can be empty if the matchmaker was only able to partially fill the match.
     */
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Matchmaking")
    bool bHasUser = false;

    /**
     * If bHasUser is true, this is the user ID for this slot.
     */
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Matchmaking")
    FUniqueNetIdRepl UserId;
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()