// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "GameFramework/OnlineReplStructs.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

#include "DedicatedServerMatchmakingBeaconClientMatchMember.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

USTRUCT()
struct MATCHMAKING_API FDedicatedServerMatchmakingBeaconClientMatchMember
{
    GENERATED_BODY()

public:
    UPROPERTY()
    FUniqueNetIdRepl UserId;

    UPROPERTY()
    int32 Team = 0;

    UPROPERTY()
    int32 Slot = 0;

    UPROPERTY()
    int8 SlotType = 0;
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()