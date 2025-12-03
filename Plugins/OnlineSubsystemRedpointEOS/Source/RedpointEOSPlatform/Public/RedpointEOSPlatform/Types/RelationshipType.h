// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/FlagEnum.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3519545918, Redpoint::EOS::Platform::Types)
{

/**
 * Flag enumeration representing one or more types of relationships that a local user has to a remote user.
 */
enum class ERelationshipType : uint8
{
    None = 0x0,

    /**
     * The local user and remote user are mutual friends.
     */
    MutualFriends = 0x1,

    /**
     * The remote user has sent the local user a friend request that is yet to be accepted or declined.
     */
    IncomingFriendRequest = 0x2,

    /**
     * The local user has sent the remote user a friend request that is yet to be accepted or declined.
     */
    OutgoingFriendRequest = 0x4,

    /**
     * The local user has blocked the remote user.
     */
    Blocked = 0x8,

    /**
     * The local user has recently played with this remote user in a multiplayer game.
     */
    RecentPlayer = 0xF,
};

REDPOINT_EOS_DECLARE_FLAG_ENUMERATION_OPERATORS(ERelationshipType)

}

namespace Redpoint::EOS::Platform::Types
{
REDPOINT_EOS_FILE_NS_EXPORT(3519545918, Redpoint::EOS::Platform::Types, ERelationshipType)
}

REDPOINT_EOS_CODE_GUARD_END()