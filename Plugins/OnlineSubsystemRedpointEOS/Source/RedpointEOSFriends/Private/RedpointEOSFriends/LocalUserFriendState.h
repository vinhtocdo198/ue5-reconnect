// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "RedpointEOSCore/Id/AccountIdMap.h"
#include "RedpointEOSFriends/Database/SerializedFriendDatabase.h"
#include "RedpointEOSIdentity/IdentityUserId.h"
#include "RedpointEOSPlatform/RuntimePlatformIntegration.h"
#include "RedpointEOSPresence/PresenceUserState.h"
#include "RedpointEOSUserCache/ExternalUserInfo.h"
#include "RedpointEOSUserCache/UserInfo.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3056023587, Redpoint::EOS::Friends)
{
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::Friends::Database;
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::Platform;
using namespace ::Redpoint::EOS::Presence;
using namespace ::Redpoint::EOS::UserCache;

class FIncomingLocalUserFriendState
{
public:
    TAccountIdMap<FUserInfoRef> UnifiedFriends;
    TArray<FExternalUserInfoRef> NonUnifiedFriends;
    // @note: FUserInfoPtr may be null if we only have the PUID (but couldn't look them up).
    TAccountIdMap<FUserInfoPtr> BlockedPlayers;
    TAccountIdMap<TTuple<FUserInfoRef, FDateTime>> RecentPlayers;
    TAccountIdMap<FUserInfoRef> InboundFriendRequests;
    TAccountIdMap<FUserInfoRef> OutboundFriendRequests;

    FIncomingLocalUserFriendState();
};

class FLocalUserFriendState : public FIncomingLocalUserFriendState
{
public:
    bool bRequireNoDatabaseFilePresent;
    TSharedRef<FSerializedFriendDatabase> FriendDatabase;
    bool bDirty;
    TMap<TSharedRef<IRuntimePlatformIntegration>, FDelegateHandle> OnRelationshipsChangedHandles;
    TMap<TSharedRef<IRuntimePlatformIntegration>, FDelegateHandle> OnNonUnifiedFriendPresenceChangedHandles;
    bool bHasCacheUpdateEverDirtiedDatabase;

    TSet<FIdentityUserId> PresenceInterestList;

    TAccountIdMap<FPresenceUserState> UnifiedFriendsPresenceUserState;
    TMap<TTuple<EOS_EExternalAccountType, FString>, EPresenceUserActivityState>
        NonUnifiedFriendsPresenceUserActivityState;

    FLocalUserFriendState(const TSharedRef<FSerializedFriendDatabase> &InFriendDatabase);
    FLocalUserFriendState(bool bRequireNoDatabaseFilePresent);
};

}

namespace Redpoint::EOS::Friends
{
REDPOINT_EOS_FILE_NS_EXPORT(3056023587, Redpoint::EOS::Friends, FIncomingLocalUserFriendState)
REDPOINT_EOS_FILE_NS_EXPORT(3056023587, Redpoint::EOS::Friends, FLocalUserFriendState)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()