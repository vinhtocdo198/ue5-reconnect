// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSUserCache/UserCacheSystemImpl.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3993495197, Redpoint::EOS::UserCache)
{

FUserCacheSystemImpl::FUserCacheSystemImpl(
    const FPlatformHandle &InPlatformHandle,
    const FPlatformInstanceEnvironmentRef &InPlatformInstanceEnvironment)
    : PlatformHandle(InPlatformHandle)
    , PlatformInstanceEnvironment(InPlatformInstanceEnvironment)
    , Caches()
{
}

TSharedRef<FUserCache> FUserCacheSystemImpl::GetCache(const UE::Online::FAccountId &InLocalUserId)
{
    auto &ExistingCache = this->Caches.FindOrAdd(InLocalUserId);
    if (!ExistingCache.IsValid())
    {
        ExistingCache = MakeShared<FUserCache>(this->PlatformHandle, this->PlatformInstanceEnvironment, InLocalUserId);
    }
    return ExistingCache.ToSharedRef();
}

void FUserCacheSystemImpl::GetUser(
    const UE::Online::FAccountId &InLocalUserId,
    const UE::Online::FAccountId &InTargetUserId,
    const FOnGetUserComplete &InOnComplete)
{
    this->GetCache(InLocalUserId)->GetUser(InTargetUserId, InOnComplete);
}

void FUserCacheSystemImpl::GetUsers(
    const UE::Online::FAccountId &InLocalUserId,
    const TArray<UE::Online::FAccountId> &InTargetUserIds,
    const FOnGetUsersComplete &InOnComplete)
{
    this->GetCache(InLocalUserId)->GetUsers(InTargetUserIds, InOnComplete);
}

FUserInfoRef FUserCacheSystemImpl::GetUnresolvedUser(
    const UE::Online::FAccountId &InLocalUserId,
    const UE::Online::FAccountId &InTargetUserId)
{
    return this->GetCache(InLocalUserId)->GetUnresolvedUser(InTargetUserId);
}

void FUserCacheSystemImpl::GetUsersByEpicGamesDisplayNames(
    const UE::Online::FAccountId &InLocalUserId,
    const TArray<FString> &InDisplayNames,
    const FOnGetUsersByEpicGamesDisplayNamesComplete &InOnComplete)
{
    this->GetCache(InLocalUserId)->GetUsersByEpicGamesDisplayNames(InDisplayNames, InOnComplete);
}

void FUserCacheSystemImpl::GetUsersByExternalAccountIds(
    const UE::Online::FAccountId &InLocalUserId,
    EOS_EExternalAccountType InExternalAccountType,
    const TArray<FString> &InExternalAccountIds,
    const FOnGetUsersByExternalAccountIdsComplete &InOnComplete)
{
    this->GetCache(InLocalUserId)
        ->GetUsersByExternalAccountIds(InExternalAccountType, InExternalAccountIds, InOnComplete);
}

void FUserCacheSystemImpl::ClearCache(const UE::Online::FAccountId &InLocalUserId)
{
    this->GetCache(InLocalUserId)->ClearCache();
}

}

REDPOINT_EOS_CODE_GUARD_END()