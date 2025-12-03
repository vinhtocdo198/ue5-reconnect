// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSCore/Id/AccountIdMap.h"
#include "RedpointEOSFriends/LocalUserFriendState.h"
#include "RedpointEOSFriends/Unification/UnifiedUserInfo.h"
#include "RedpointEOSIdentity/IdentityUserId.h"
#include "RedpointEOSPlatform/RuntimePlatformIntegration.h"
#include "RedpointEOSPlatform/Services/RuntimePlatformServiceCallContext.h"
#include "RedpointEOSUserCache/ExternalUserInfo.h"
#include "RedpointEOSUserCache/UserInfo.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3009419599, Redpoint::EOS::Friends::Unification)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::Friends;
using namespace ::Redpoint::EOS::Friends::Unification;
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::Platform;
using namespace ::Redpoint::EOS::Platform::Services;
using namespace ::Redpoint::EOS::UserCache;

class FFriendUnification
{
private:
    /**
     * A cache key as returned by IRuntimePlatformRelationshipService.
     */
    typedef FString FCacheKey;

    /**
     * A tuple of the external user info and optional avatar URL pulled from a runtime integration. ExternalUserInfo
     * is only a Ptr to make this type safe as a TMap value; all real values of this struct should have ExternalUserInfo
     * populated with a real value.
     */
    struct FExternalUserInfoAndAvatarUrl
    {
        FExternalUserInfoPtr ExternalUserInfo;
        TOptional<FString> AvatarUrl;
    };

    /**
     * A tuple of an external account type and account ID that we can use as a TMap key.
     */
    typedef TTuple<EOS_EExternalAccountType, FString> FExternalAccountTypeAndId;

    /**
     * A tuple of a user info (i.e. product user ID) and optional avatar URL pulled from the IUserCacheSystem. UserInfo
     * is only a Ptr to make this type safe as a TMap value; all real values of this struct should have UserInfo
     * populated with a real value.
     */
    struct FUserInfoAndAvatarUrl
    {
        FUserInfoPtr UserInfo;
        TOptional<FString> AvatarUrl;
    };

    /**
     * Tracks information about a unified friend as we assemble all the information together prior to constructing
     * FUnifiedUserInfo.
     */
    struct FUnifiedFriendPartialState
    {
        FUserInfoPtr PuidUserInfo;
        TMap<FExternalAccountTypeAndId, FExternalUserInfoAndAvatarUrl> ExternalUserInfos;
        TOptional<FString> AvatarUrl;
    };

    /**
     * The state tracked as we perform friend unification.
     */
    struct FFriendUnificationState
    {
        /**
         * A map of runtime integration relationship service cache keys to the list of external mutual friends they
         * provided, and the avatars associated with those users for the runtime integration they were loaded from.
         *
         * The data in this map will look something like this:
         * - Steam:
         *   - Steam friend 1 + avatar URL
         *   - Steam friend 2 + avatar URL
         * - Discord:
         *   - Discord friend 1 + avatar URL
         *   - Discord friend 2 + avatar URL
         *
         * Only mutual friends are retrieved, as it is not possible for games to interact with platform friend invites
         * in most cases.
         *
         * @note: After external friends are loaded from the cross-platform friends database cache, this map is *also*
         * populated with cache keys to FCachedExternalUserInfo objects representing those friends.
         */
        TMap<FCacheKey, TArray<FExternalUserInfoAndAvatarUrl>> ExternalFriendsWithAvatarsByCacheKey;

        /**
         * For platforms that we could not load external friends through a runtime integration for, but for whom a
         * cached list exists in the cross-platform friends database, this is a set of the FCachedExternalUserInfo
         * instances created when loading from the cache so that we can safely upcast TSharedRef<FExternalUserInfo> into
         * them later.
         *
         * If the local player was playing on Steam, this list might consist of FCachedExternalUserInfo representing
         * Google Play friends.
         */
        TSet<TSharedRef<FExternalUserInfo>> ExternalFriendsLoadedFromCache;

        /**
         * The set of runtime integration relationship service cache keys where we loaded friends from the
         * cross-platform friends database cache, instead of via a runtime integration.
         */
        TSet<FCacheKey> CacheKeysLoadedFromCache;

        /**
         * A map of external account types and account IDs to the FExternalUserInfoAndAvatarUrl. This map is set
         * immediately before querying for PUIDs, and indicates an authoritative mapping of external account types and
         * account IDs to an FExternalUserInfoAndAvatarUrl for the purpose of attaching FExternalUserInfo to an
         * FUnifiedUserInfo later.
         *
         * This is necessary because, although unlikely, it's possible for multiple runtime platform integrations to
         * return external user infos for the same external account, and we need to pick one of them now for associating
         * to friend information later.
         */
        TMap<FExternalAccountTypeAndId, FExternalUserInfoAndAvatarUrl> ExternalAccounts;

        /**
         * A map of external account types and account IDs to the full user info loaded from IUserCacheSystem. This map
         * represents friends that we retrieved from runtime integrations or the cross-platform friends cache that we
         * were then able to elevate into full product user IDs.
         *
         * This map also tracks the avatar URL for user infos - for any external friends that came from the
         * cross-platform friend database cache, we want to keep that avatar info after loading the PUID so that we can
         * offer that URL in an "avatarUrl" attribute that IOnlineAvatar can then use. For example, you could be playing
         * on itch.io, have a Steam friend as a cached external friend that has a PUID, and we want to display the Steam
         * avatar for that PUID. The only way for that to work is track the avatar URL against external accounts/PUIDs
         * here.
         */
        TMap<FExternalAccountTypeAndId, FUserInfoAndAvatarUrl> ExternalAccountsToFullAccounts;

        /**
         * A map of unified mutual friends.
         */
        TAccountIdMap<FUnifiedUserInfoPtr> UnifiedFriends;

        /**
         * A map of non-unified mutual friends.
         */
        TArray<FExternalUserInfoRef> NonUnifiedFriends;
    };

public:
    typedef TDelegate<void(
        const FError &ErrorCode,
        const TAccountIdMap<FUnifiedUserInfoPtr> &UnifiedFriends,
        const TArray<FExternalUserInfoRef> &NonUnifiedFriends)>
        FOnReadUnifiedFriendsComplete;

    static void ReadUnifiedFriends(
        const FPlatformHandle &PlatformHandle,
        FIdentityUserId LocalUserId,
        const TSharedRef<FLocalUserFriendState> &LocalUserFriendState,
        const TArray<TSharedRef<IRuntimePlatformIntegration>> &RuntimePlatformIntegrations,
        const FOnReadUnifiedFriendsComplete &OnComplete);
};

}

namespace Redpoint::EOS::Friends::Unification
{
REDPOINT_EOS_FILE_NS_EXPORT(3009419599, Redpoint::EOS::Friends::Unification, FFriendUnification);
}

#endif

REDPOINT_EOS_CODE_GUARD_END()