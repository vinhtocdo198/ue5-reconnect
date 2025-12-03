// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Connect/QueryProductUserIdMappings.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Utils/ExclusiveAsyncOperationQueue.h"
#include "RedpointEOSUserCache/UserCacheSystem.h"
#include "RedpointEOSUserCache/UserInfoImpl.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(375513567, Redpoint::EOS::UserCache)
{
using namespace ::Redpoint::EOS;
using namespace ::Redpoint::EOS::Core::Utils;
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::UserCache;

template <typename TKey, typename TValue> class TCacheTypes
{
public:
    typedef TKey FKeyType;
    typedef TValue FValueType;

    class FPendingCallback
    {
    public:
        typedef TDelegate<void(const FPendingCallback &Pending)> FOnResolved;

        FPendingCallback(const TArray<TKey> &InRequiredKeys, const FOnResolved &InOnResolved)
            : RequiredKeys(InRequiredKeys)
            , OnResolved(InOnResolved)
        {
        }

        const TArray<TKey> RequiredKeys;
        const FOnResolved OnResolved;
    };

    class FCacheEntry
    {
    public:
        FCacheEntry(const TKey &InKey, const TValue &InValue)
            : Value(InValue)
            , Error(nullptr)
        {
        }

        FCacheEntry(const TKey &InKey)
            : Value(MakeShared<FUserInfoImpl>(InKey))
            , Error(nullptr)
        {
        }

        TValue Value;
        TSharedPtr<FError> Error;
    };
    typedef TSharedPtr<FCacheEntry> FCacheEntryPtr;

    typedef TMap<TKey, FCacheEntryPtr> FCacheMap;
    typedef TPair<TKey, FCacheEntryPtr> FCachePair;
    typedef TSet<TSharedPtr<FPendingCallback>> FPendingCallbackMap;
};

typedef TCacheTypes<UE::Online::FAccountId, TSharedRef<FUserInfoImpl>> FUsersCache;
typedef TCacheTypes<FString, TOptional<FString>> FEpicAccountIdsByDisplayNameCache;
typedef TCacheTypes<TTuple<EOS_EExternalAccountType, FString>, TOptional<UE::Online::FAccountId>>
    FUserIdsByExternalAccountIdCache;

class FUserCache : public TSharedFromThis<FUserCache>
{
private:
    FPlatformHandle PlatformHandle;
    FPlatformInstanceEnvironmentRef PlatformInstanceEnvironment;
    UE::Online::FAccountId LocalUserId;
    TSharedRef<FExclusiveAsyncOperationQueue> OperationQueue;

    FUsersCache::FCacheMap UsersCached;
    FUsersCache::FPendingCallbackMap UsersPendingCallback;
    FEpicAccountIdsByDisplayNameCache::FCacheMap EpicAccountIdsByDisplayNameCached;
    FEpicAccountIdsByDisplayNameCache::FPendingCallbackMap EpicAccountIdsByDisplayNamePendingCallback;
    FUserIdsByExternalAccountIdCache::FCacheMap UserIdsByExternalAccountIdCached;
    FUserIdsByExternalAccountIdCache::FPendingCallbackMap UserIdsByExternalAccountIdPendingCallback;

    EOS_ProductUserId GetLocalUserIdParameter() const;

public:
    FUserCache(
        const FPlatformHandle &InPlatformHandle,
        const FPlatformInstanceEnvironmentRef &InPlatformInstanceEnvironment,
        const UE::Online::FAccountId &InLocalUserId);
    UE_NONCOPYABLE(FUserCache);
    ~FUserCache() = default;

    void GetUser(
        const UE::Online::FAccountId &InTargetUserId,
        const IUserCacheSystem::FOnGetUserComplete &InOnComplete);
    void GetUsers(
        const TArray<UE::Online::FAccountId> &InTargetUserIds,
        const IUserCacheSystem::FOnGetUsersComplete &InOnComplete);
    FUserInfoRef GetUnresolvedUser(const UE::Online::FAccountId &InTargetUserId);
    void GetUsersByEpicGamesDisplayNames(
        const TArray<FString> &InDisplayNames,
        const IUserCacheSystem::FOnGetUsersByEpicGamesDisplayNamesComplete &InOnComplete);
    void GetUsersByExternalAccountIds(
        EOS_EExternalAccountType InExternalAccountType,
        const TArray<FString> &InExternalAccountIds,
        const IUserCacheSystem::FOnGetUsersByExternalAccountIdsComplete &InOnComplete);
    void ClearCache();

private:
    void OnUsersResolvedForGetUser();

    void Queue_FlushPendingCallbacksAndPotentiallySchedule();

    void UsersQueue_Process(const FExclusiveAsyncOperationQueue::FOnDone &InOnDone);
    void UsersQueue_OnProductUserIdMappingsQueried(
        const API::Connect::FQueryProductUserIdMappings::Result &InResult,
        TSet<UE::Online::FAccountId> InUsersToResolve,
        FExclusiveAsyncOperationQueue::FOnDone InOnDone);

    void EpicAccountIdsByDisplayNameQueue_Process(const FExclusiveAsyncOperationQueue::FOnDone &InOnDone);

    void UserIdsByExternalAccountIdQueue_Process(const FExclusiveAsyncOperationQueue::FOnDone &InOnDone);
};

}

namespace Redpoint::EOS::UserCache
{
REDPOINT_EOS_FILE_NS_EXPORT(375513567, Redpoint::EOS::UserCache, TCacheTypes)
REDPOINT_EOS_FILE_NS_EXPORT(375513567, Redpoint::EOS::UserCache, FUsersCache)
REDPOINT_EOS_FILE_NS_EXPORT(375513567, Redpoint::EOS::UserCache, FEpicAccountIdsByDisplayNameCache)
REDPOINT_EOS_FILE_NS_EXPORT(375513567, Redpoint::EOS::UserCache, FUserIdsByExternalAccountIdCache)
REDPOINT_EOS_FILE_NS_EXPORT(375513567, Redpoint::EOS::UserCache, FUserCache)
}

REDPOINT_EOS_CODE_GUARD_END()