// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSUserCache/UserCache.h"
#include "RedpointEOSUserCache/UserCacheSystem.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3993495197, Redpoint::EOS::UserCache)
{
using namespace ::Redpoint::EOS::UserCache;
using namespace ::Redpoint::EOS::API;

class REDPOINTEOSUSERCACHE_API FUserCacheSystemImpl : public IUserCacheSystem
{
private:
    FPlatformHandle PlatformHandle;
    FPlatformInstanceEnvironmentRef PlatformInstanceEnvironment;
    TMap<UE::Online::FAccountId, TSharedPtr<FUserCache>> Caches;
    TSharedRef<FUserCache> GetCache(const UE::Online::FAccountId &InLocalUserId);

public:
    FUserCacheSystemImpl(
        const FPlatformHandle &InPlatformHandle,
        const FPlatformInstanceEnvironmentRef &InPlatformInstanceEnvironment);
    UE_NONCOPYABLE(FUserCacheSystemImpl);
    virtual ~FUserCacheSystemImpl() override = default;

    virtual void GetUser(
        const UE::Online::FAccountId &InLocalUserId,
        const UE::Online::FAccountId &InTargetUserId,
        const FOnGetUserComplete &InOnComplete) override;
    virtual void GetUsers(
        const UE::Online::FAccountId &InLocalUserId,
        const TArray<UE::Online::FAccountId> &InTargetUserIds,
        const FOnGetUsersComplete &InOnComplete) override;
    virtual FUserInfoRef GetUnresolvedUser(
        const UE::Online::FAccountId &InLocalUserId,
        const UE::Online::FAccountId &InTargetUserId) override;
    virtual void GetUsersByEpicGamesDisplayNames(
        const UE::Online::FAccountId &InLocalUserId,
        const TArray<FString> &InDisplayNames,
        const FOnGetUsersByEpicGamesDisplayNamesComplete &InOnComplete) override;
    virtual void GetUsersByExternalAccountIds(
        const UE::Online::FAccountId &InLocalUserId,
        EOS_EExternalAccountType InExternalAccountType,
        const TArray<FString> &InExternalAccountIds,
        const FOnGetUsersByExternalAccountIdsComplete &InOnComplete) override;
    virtual void ClearCache(const UE::Online::FAccountId &InLocalUserId) override;
};

}

namespace Redpoint::EOS::UserCache
{
REDPOINT_EOS_FILE_NS_EXPORT(3993495197, Redpoint::EOS::UserCache, FUserCacheSystemImpl)
}

REDPOINT_EOS_CODE_GUARD_END()