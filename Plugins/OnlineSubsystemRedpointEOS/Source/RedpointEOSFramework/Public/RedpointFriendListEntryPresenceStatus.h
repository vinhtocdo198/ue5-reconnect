// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointFriendListEntryPresenceStatus.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

UENUM(BlueprintType, Category = "Redpoint EOS Online Framework|Friend Lists for UMG")
enum class ERedpointFriendListEntryPresenceStatus : uint8
{
    /**
     * The user's presence status is unknown, which could be the case if the user is blocked. In this case, no presence
     * status or information should be displayed for the user (i.e. you should not display "Unknown").
     */
    Unknown,

    /**
     * The user is offline or is not advertising presence.
     */
    Offline,

    /**
     * The user is busy or has turned on do-not-disturb.
     */
    DoNotDisturb,

    /**
     * The user is idle and hasn't sent keyboard/mouse/controller input to the game in a while.
     */
    Idle,

    /**
     * The user is online and active.
     */
    Online,
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()
