// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "RedpointEOSCore/Id/AccountIdMap.h"
#include "RedpointEOSFriends/Database/SerializedBlockedUser.h"
#include "RedpointEOSFriends/Database/SerializedCachedFriend.h"
#include "RedpointEOSFriends/Database/SerializedExternalAccount.h"
#include "RedpointEOSFriends/Database/SerializedFriend.h"
#include "RedpointEOSFriends/Database/SerializedPendingFriendRequest.h"
#include "RedpointEOSFriends/Database/SerializedRecentPlayer.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3612751533, Redpoint::EOS::Friends::Database)
{
using namespace ::Redpoint::EOS::Friends::Database;
using namespace ::Redpoint::EOS::Core::Id;

class FSerializedFriendDatabase
{
public:
    /**
     * Friends that have been explicitly added by the user. These are never removed unless the user
     * explicitly removes them from their friend list.
     */
    TAccountIdMap<FSerializedFriend> Friends;

    /**
     * A copy of the friends from runtime integrations is stored in the friend database. This allows us to pull metadata
     * about friends the user has on platforms they aren't signed into at the moment (for example, we can pull
     * information about Steam friends while playing on console).
     */
    TMap<FString, TArray<FSerializedCachedFriend>> RuntimeIntegrationCachedFriends;

    /**
     * A list of players the user has recently interacted with.
     */
    TAccountIdMap<FSerializedRecentPlayer> RecentPlayers;

    /**
     * A list of blocked users.
     */
    TAccountIdMap<FSerializedBlockedUser> BlockedUsers;

    /**
     * A list of pending friend requests. This includes friends requests we are yet to send (because
     * the target is offline), friend requests we're waiting to get a response on, and friend responses
     * we're yet to send (because the original requestor is offline).
     */
    TAccountIdMap<FSerializedPendingFriendRequest> PendingFriendRequests;

    /**
     * A map of user IDs to friend aliases.
     */
    TAccountIdMap<FString> FriendAliases;

    /**
     * Serialize/deserialize state.
     */
    void Archive(FArchive &Ar);
};

}

namespace Redpoint::EOS::Friends::Database
{
REDPOINT_EOS_FILE_NS_EXPORT(3612751533, Redpoint::EOS::Friends::Database, FSerializedFriendDatabase);
}

#endif

REDPOINT_EOS_CODE_GUARD_END()