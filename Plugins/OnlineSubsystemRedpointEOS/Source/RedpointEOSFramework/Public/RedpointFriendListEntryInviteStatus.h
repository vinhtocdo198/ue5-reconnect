// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointFriendListEntryInviteStatus.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

UENUM(BlueprintType, Category = "Redpoint EOS Online Framework|Friend Lists for UMG")
enum class ERedpointFriendListEntryInviteStatus : uint8
{
    /**
     * The user's invite status is unknown. In this case, no actions or invite status should be displayed for the user
     * (i.e. you should not display "Unknown").
     */
    Unknown,

    /**
     * The user is a mutual friend.
     */
    MutualFriends,

    /**
     * The user has sent a friend request to the local player, which needs to be accepted or rejected.
     */
    PendingInbound,

    /**
     * The local player has sent the user a friend request, but they have not accepted or rejected it yet.
     */
    PendingOutbound,

    /**
     * The user is unrelated to the local player.
     */
    Unrelated,

    /**
     * The user recently played on a multiplayer server with the local player.
     */
    Recent,

    /**
     * The user has been blocked by the local player.
     */
    Blocked,
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()
