// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Online/CoreOnline.h"
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSUserCache/ErrorableUserInfo.h"
#include "RedpointEOSUserCache/UserInfo.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1660508860, Redpoint::EOS::UserCache)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::UserCache;

class REDPOINTEOSUSERCACHE_API IUserCacheSystem : public IPlatformInstanceSystem
{
public:
    static FName GetSystemName();
    static TSharedRef<IUserCacheSystem> ConstructSystem(
        FPlatformHandle InPlatformHandle,
        FPlatformInstanceEnvironmentRef InPlatformInstanceEnvironment);

public:
    IUserCacheSystem() = default;
    UE_NONCOPYABLE(IUserCacheSystem);
    virtual ~IUserCacheSystem() override = default;

    typedef TDelegate<void(const FError &ErrorCode, const FUserInfoPtr &UserInfo)> FOnGetUserComplete;
    typedef TDelegate<
        void(const FError &CallErrorCode, const TMap<UE::Online::FAccountId, FErrorableUserInfo> &UserInfos)>
        FOnGetUsersComplete;
    typedef TDelegate<void(const FError &CallErrorCode, const TMap<FString, FErrorableUserInfo> &UserInfos)>
        FOnGetUsersByEpicGamesDisplayNamesComplete;
    typedef TDelegate<void(const FError &CallErrorCode, const TMap<FString, FErrorableUserInfo> &UserInfos)>
        FOnGetUsersByExternalAccountIdsComplete;

    /**
     * Get information about a target user.
     */
    virtual void GetUser(
        const UE::Online::FAccountId &InLocalUserId,
        const UE::Online::FAccountId &InTargetUserId,
        const FOnGetUserComplete &InOnComplete) = 0;

    /**
     * Get information about multiple target users.
     */
    virtual void GetUsers(
        const UE::Online::FAccountId &InLocalUserId,
        const TArray<UE::Online::FAccountId> &InTargetUserIds,
        const FOnGetUsersComplete &InOnComplete) = 0;

    /**
     * Get a user info object for a target user, where the user information may not have been resolved yet. The state of
     * the returned FUserInfo will be automatically updated once the EOS backend has been queried.
     */
    virtual FUserInfoRef GetUnresolvedUser(
        const UE::Online::FAccountId &InLocalUserId,
        const UE::Online::FAccountId &InTargetUserId) = 0;

    /**
     * Get information about multiple target users based on their Epic Games account display names. This only works if
     * the local user is signed into an Epic Games account.
     */
    virtual void GetUsersByEpicGamesDisplayNames(
        const UE::Online::FAccountId &InLocalUserId,
        const TArray<FString> &InDisplayNames,
        const FOnGetUsersByEpicGamesDisplayNamesComplete &InOnComplete) = 0;

    /**
     * Get information about multiple target users based on their external account IDs for an external account type. Due
     * to platform restrictions, there are some platforms you can not query for unless the game is also running on that
     * platform.
     */
    virtual void GetUsersByExternalAccountIds(
        const UE::Online::FAccountId &InLocalUserId,
        EOS_EExternalAccountType InExternalAccountType,
        const TArray<FString> &InExternalAccountIds,
        const FOnGetUsersByExternalAccountIdsComplete &InOnComplete) = 0;

    /**
     * Clear all cached user objects for the specified local user.
     */
    virtual void ClearCache(const UE::Online::FAccountId &InLocalUserId) = 0;
};

}

namespace Redpoint::EOS::UserCache
{
REDPOINT_EOS_FILE_NS_EXPORT(1660508860, Redpoint::EOS::UserCache, IUserCacheSystem)
}

REDPOINT_EOS_CODE_GUARD_END()