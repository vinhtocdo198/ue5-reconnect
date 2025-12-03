// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSFriends/LocalUserFriendState.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3056023587, Redpoint::EOS::Friends)
{
using namespace ::Redpoint::EOS::Friends::Database;

FIncomingLocalUserFriendState::FIncomingLocalUserFriendState()
    : UnifiedFriends()
    , NonUnifiedFriends()
    , BlockedPlayers()
    , RecentPlayers()
    , InboundFriendRequests()
    , OutboundFriendRequests()
{
}

FLocalUserFriendState::FLocalUserFriendState(const TSharedRef<FSerializedFriendDatabase> &InFriendDatabase)
    : FIncomingLocalUserFriendState()
    , bRequireNoDatabaseFilePresent(false)
    , FriendDatabase(InFriendDatabase)
    , bDirty(false)
    , OnRelationshipsChangedHandles()
    , OnNonUnifiedFriendPresenceChangedHandles()
    , bHasCacheUpdateEverDirtiedDatabase(false)
    , PresenceInterestList()
    , UnifiedFriendsPresenceUserState()
    , NonUnifiedFriendsPresenceUserActivityState()
{
}

FLocalUserFriendState::FLocalUserFriendState(bool bInRequireNoDatabaseFilePresent)
    : FIncomingLocalUserFriendState()
    , bRequireNoDatabaseFilePresent(bInRequireNoDatabaseFilePresent)
    , FriendDatabase(MakeShared<FSerializedFriendDatabase>())
    , bDirty(false)
    , OnRelationshipsChangedHandles()
    , OnNonUnifiedFriendPresenceChangedHandles()
    , bHasCacheUpdateEverDirtiedDatabase(false)
    , PresenceInterestList()
    , UnifiedFriendsPresenceUserState()
    , NonUnifiedFriendsPresenceUserActivityState()
{
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()