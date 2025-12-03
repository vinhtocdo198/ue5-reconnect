// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointFriendListSectionType.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

UENUM(BlueprintType, Category = "Redpoint EOS Online Framework|Friend Lists for UMG")
enum class ERedpointFriendListSectionType : uint8
{
    /**
     * Friends and non-friends who are currently in the player's primary party.
     */
    InParty,

    /**
     * Friends who are currently in-game.
     */
    InGame,

    /**
     * Friends who are in any online status (not offline), with friends in an away/busy prior at the bottom of the
     * section.
     */
    Online,

    /**
     * Friends who are currently offline.
     */
    Offline,

    /**
     * Non-friends who the player has recently played with on multiplayer servers.
     */
    Recent,

    /**
     * Users who the player has blocked.
     */
    Blocked,
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()
