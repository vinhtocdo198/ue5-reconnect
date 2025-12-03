// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/User/OnlineUserInterfaceEOS.h"

#include "OnlineSubsystemRedpointEOS/Logging.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/SubsystemNames.h"
#include "RedpointEOSAPI/Common/ExternalAccountType.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSUserCache/UserCacheSystem.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "RedpointEOSPresence/PresenceSystem.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(757011916, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::User)
{
using namespace ::Redpoint::EOS::UserCache;
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::API::Common;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS;
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
using namespace ::Redpoint::EOS::Presence;
#endif

FOnlineUserInterfaceEOS::FOnlineUserInterfaceEOS(
    const TSharedRef<IOnlineIdentityInterfaceEOS> &InIdentity,
    const FPlatformHandle &InPlatformHandle)
    : Identity(InIdentity)
    , PlatformHandle(InPlatformHandle)
    , Cache()
{
}

void FOnlineUserInterfaceEOS::AddToCache(int32 LocalUserNum, const FUserInfoRef &UserInfo)
{
    auto &CacheForLocalUser = this->Cache.FindOrAdd(LocalUserNum);

    auto OnlineUserEOS = MakeShared<FOnlineUserEOS>(this->PlatformHandle->GetEnvironment()->InstanceName, UserInfo);
    CacheForLocalUser.UsersById.Add(*GetUniqueNetId(UserInfo->GetUserId()), OnlineUserEOS);
    for (const auto &KV : UserInfo->GetLinkedExternalAccounts())
    {
        auto &CacheForLocalUserExternalType = CacheForLocalUser.UsersByExternalId.FindOrAdd(KV.Key);
        CacheForLocalUserExternalType.Add(KV.Value.AccountId, OnlineUserEOS);
    }

    if (UserInfo->GetLinkedEpicGamesAccount().IsSet())
    {
        CacheForLocalUser.UsersByEpicGamesDisplayName.Add(
            UserInfo->GetLinkedEpicGamesAccount().GetValue().DisplayName,
            OnlineUserEOS);
    }
}

TSharedPtr<FOnlineUserEOS> FOnlineUserInterfaceEOS::GetFromCache(int32 LocalUserNum, const FUniqueNetId &UserId) const
{
    auto *CacheForLocalUser = this->Cache.Find(LocalUserNum);
    if (CacheForLocalUser == nullptr)
    {
        return nullptr;
    }

    auto *OnlineUserEOS = CacheForLocalUser->UsersById.Find(UserId.AsShared());
    if (OnlineUserEOS == nullptr)
    {
        return nullptr;
    }

    return *OnlineUserEOS;
}

TSharedPtr<FOnlineUserEOS> FOnlineUserInterfaceEOS::GetFromCacheByExternalAccountId(
    int32 LocalUserNum,
    EOS_EExternalAccountType ExternalAccountType,
    const FString &ExternalAccountId) const
{
    auto *CacheForLocalUser = this->Cache.Find(LocalUserNum);
    if (CacheForLocalUser == nullptr)
    {
        return nullptr;
    }

    auto *CacheForLocalUserExternalType = CacheForLocalUser->UsersByExternalId.Find(ExternalAccountType);
    if (CacheForLocalUserExternalType == nullptr)
    {
        return nullptr;
    }

    auto *OnlineUserEOS = CacheForLocalUserExternalType->Find(ExternalAccountId);
    if (OnlineUserEOS == nullptr)
    {
        return nullptr;
    }

    return *OnlineUserEOS;
}

TSharedPtr<FOnlineUserEOS> FOnlineUserInterfaceEOS::GetFromCacheByEpicGamesDisplayName(
    int32 LocalUserNum,
    const FString &EpicGamesDisplayName) const
{
    auto *CacheForLocalUser = this->Cache.Find(LocalUserNum);
    if (CacheForLocalUser == nullptr)
    {
        return nullptr;
    }

    auto *OnlineUserEOS = CacheForLocalUser->UsersByEpicGamesDisplayName.Find(EpicGamesDisplayName);
    if (OnlineUserEOS == nullptr)
    {
        return nullptr;
    }

    return *OnlineUserEOS;
}

bool FOnlineUserInterfaceEOS::QueryUserInfo(int32 LocalUserNum, const TArray<TSharedRef<const FUniqueNetId>> &UserIds)
{
    if (this->Identity->GetLoginStatus(LocalUserNum) != ELoginStatus::LoggedIn)
    {
        UE_LOG(LogOnlineSubsystemRedpointEOS, Error, TEXT("QueryUserInfo: Local user is not logged in"));
        return false;
    }

    auto LocalUserId = GetAccountId(*this->Identity->GetUniquePlayerId(LocalUserNum));
    TArray<UE::Online::FAccountId> TargetUserIds;
    for (const auto &UserId : UserIds)
    {
        if (UserId->GetType() == REDPOINT_EOS_SUBSYSTEM)
        {
            TargetUserIds.Add(GetAccountId(UserId));
        }
    }

    auto UserCache = this->PlatformHandle->GetSystem<IUserCacheSystem>();
    UserCache->GetUsers(
        LocalUserId,
        TargetUserIds,
        IUserCacheSystem::FOnGetUsersComplete::CreateSPLambda(
            this,
            [this,
             LocalUserNum,
             UserIds](FError CallErrorCode, TMap<UE::Online::FAccountId, FErrorableUserInfo> UserInfos) {
                if (!CallErrorCode.WasSuccessful())
                {
                    this->TriggerOnQueryUserInfoCompleteDelegates(
                        LocalUserNum,
                        false,
                        UserIds,
                        CallErrorCode.ToLogString());
                    return;
                }

                for (const auto &ErrorableUserInfo : UserInfos)
                {
                    if (ErrorableUserInfo.Value.IsErrored())
                    {
                        continue;
                    }

                    auto UserInfo = ErrorableUserInfo.Value.GetUserInfo();
                    this->AddToCache(LocalUserNum, UserInfo);
                }
                this->TriggerOnQueryUserInfoCompleteDelegates(LocalUserNum, true, UserIds, TEXT(""));
            }));
    return true;
}

bool FOnlineUserInterfaceEOS::GetAllUserInfo(int32 LocalUserNum, TArray<TSharedRef<FOnlineUser>> &OutUsers)
{
    auto *CacheForLocalUser = this->Cache.Find(LocalUserNum);
    if (CacheForLocalUser == nullptr)
    {
        OutUsers.Empty();
        return true;
    }

    for (const auto &KV : CacheForLocalUser->UsersById)
    {
        OutUsers.Add(KV.Value.ToSharedRef());
    }
    return true;
}

TSharedPtr<FOnlineUser> FOnlineUserInterfaceEOS::GetUserInfo(int32 LocalUserNum, const FUniqueNetId &UserId)
{
    return this->GetFromCache(LocalUserNum, UserId);
}

bool FOnlineUserInterfaceEOS::QueryUserIdMapping(
    const FUniqueNetId &UserId,
    const FString &DisplayNameOrEmail,
    const FOnQueryUserMappingComplete &Delegate)
{
    int32 LocalUserNum;
    if (this->Identity->GetLoginStatus(UserId) != ELoginStatus::LoggedIn ||
        !this->Identity->GetLocalUserNum(UserId, LocalUserNum))
    {
        Delegate.ExecuteIfBound(
            false,
            UserId,
            DisplayNameOrEmail,
            *FUniqueNetIdEOS::InvalidId(),
            TEXT("Local user is not signed in."));
        return true;
    }

    auto LocalUserId = GetAccountId(UserId);

    if (DisplayNameOrEmail.StartsWith(TEXT("FriendCode:")))
    {
#if REDPOINT_EOS_IS_DEDICATED_SERVER
        Delegate.ExecuteIfBound(
            false,
            UserId,
            DisplayNameOrEmail,
            *FUniqueNetIdEOS::InvalidId(),
            TEXT("Users can not be queried by friend code on dedicated servers."));
        return true;
#else
        auto FriendCode = DisplayNameOrEmail.Mid(FString(TEXT("FriendCode:")).Len());
        auto Presence = this->PlatformHandle->GetSystem<IPresenceSystem>();
        Presence->FindByFriendCode(
            LocalUserId,
            FriendCode,
            IPresenceSystem::FOnFindByFriendCodeComplete::CreateSPLambda(
                this,
                [this, UserIdShared = UserId.AsShared(), LocalUserNum, DisplayNameOrEmail, FriendCode, Delegate](
                    const FError &CallErrorCode,
                    const TArray<FUserInfoRef> &DiscoveredUsers) {
                    if (!CallErrorCode.WasSuccessful() || DiscoveredUsers.Num() == 0)
                    {
                        Delegate.ExecuteIfBound(
                            CallErrorCode.WasSuccessful(),
                            *UserIdShared,
                            DisplayNameOrEmail,
                            *FUniqueNetIdEOS::InvalidId(),
                            CallErrorCode.WasSuccessful() ? TEXT("") : CallErrorCode.ToLogString());
                        return;
                    }

                    if (DiscoveredUsers.Num() > 1)
                    {
                        UE_LOG(
                            LogOnlineSubsystemRedpointEOS,
                            Warning,
                            TEXT("Multiple users were found for friend code '%s', but the "
                                 "IOnlineUser::QueryUserIdMapping API only allows us to return one user ID. The first "
                                 "user will be returned. To retrieve all matching users, use the "
                                 "IPresenceSystem::FindByFriendCode API directly."),
                            *FriendCode);
                    }

                    this->AddToCache(LocalUserNum, DiscoveredUsers[0]);
                    Delegate.ExecuteIfBound(
                        true,
                        *UserIdShared,
                        DisplayNameOrEmail,
                        *GetUniqueNetId(DiscoveredUsers[0]->GetUserId()),
                        TEXT(""));
                }));
#endif
    }
    else
    {
        TArray<FString> DisplayNames{DisplayNameOrEmail};
        auto UserCache = this->PlatformHandle->GetSystem<IUserCacheSystem>();
        UserCache->GetUsersByEpicGamesDisplayNames(
            LocalUserId,
            DisplayNames,
            IUserCacheSystem::FOnGetUsersByEpicGamesDisplayNamesComplete::CreateSPLambda(
                this,
                [this, UserIdShared = UserId.AsShared(), LocalUserNum, DisplayNameOrEmail, Delegate](
                    FError CallErrorCode,
                    TMap<FString, FErrorableUserInfo> UserInfos) {
                    if (!CallErrorCode.WasSuccessful())
                    {
                        Delegate.ExecuteIfBound(
                            false,
                            *UserIdShared,
                            DisplayNameOrEmail,
                            *FUniqueNetIdEOS::InvalidId(),
                            CallErrorCode.ToLogString());
                        return;
                    }
                    checkf(
                        UserInfos.Contains(DisplayNameOrEmail),
                        TEXT("Expected UserInfos to contain queried display name."));

                    auto ErrorableUserInfo = UserInfos[DisplayNameOrEmail];
                    if (ErrorableUserInfo.IsErrored())
                    {
                        Delegate.ExecuteIfBound(
                            false,
                            *UserIdShared,
                            DisplayNameOrEmail,
                            *FUniqueNetIdEOS::InvalidId(),
                            ErrorableUserInfo.GetError()->ToLogString());
                        return;
                    }

                    this->AddToCache(LocalUserNum, ErrorableUserInfo.GetUserInfo());
                    Delegate.ExecuteIfBound(
                        true,
                        *UserIdShared,
                        DisplayNameOrEmail,
                        *GetUniqueNetId(ErrorableUserInfo.GetUserInfo()->GetUserId()),
                        TEXT(""));
                }));
    }
    return true;
}

bool FOnlineUserInterfaceEOS::QueryExternalIdMappings(
    const FUniqueNetId &QueryingUserId,
    const FExternalIdQueryOptions &QueryOptions,
    const TArray<FString> &ExternalIds,
    const FOnQueryExternalIdMappingsComplete &Delegate)
{
    int32 LocalUserNum;
    if (this->Identity->GetLoginStatus(QueryingUserId) != ELoginStatus::LoggedIn ||
        !this->Identity->GetLocalUserNum(QueryingUserId, LocalUserNum))
    {
        Delegate.ExecuteIfBound(false, QueryingUserId, QueryOptions, ExternalIds, TEXT("Local user is not signed in."));
        return true;
    }
    if (QueryOptions.bLookupByDisplayName)
    {
        Delegate.ExecuteIfBound(
            false,
            QueryingUserId,
            QueryOptions,
            ExternalIds,
            TEXT("bLookupByDisplayName must be false"));
        return true;
    }

    auto LocalUserId = GetAccountId(QueryingUserId);

    auto UserCache = this->PlatformHandle->GetSystem<IUserCacheSystem>();
    UserCache->GetUsersByExternalAccountIds(
        LocalUserId,
        ExternalAccountTypeMap.Get(QueryOptions.AuthType),
        ExternalIds,
        IUserCacheSystem::FOnGetUsersByExternalAccountIdsComplete::CreateSPLambda(
            this,
            [this, UserIdShared = QueryingUserId.AsShared(), LocalUserNum, QueryOptions, ExternalIds, Delegate](
                FError CallErrorCode,
                TMap<FString, FErrorableUserInfo> UserInfos) {
                if (!CallErrorCode.WasSuccessful())
                {
                    Delegate
                        .ExecuteIfBound(false, *UserIdShared, QueryOptions, ExternalIds, CallErrorCode.ToLogString());
                    return;
                }

                for (const auto &ErrorableUserInfo : UserInfos)
                {
                    if (ErrorableUserInfo.Value.IsErrored())
                    {
                        continue;
                    }

                    auto UserInfo = ErrorableUserInfo.Value.GetUserInfo();
                    this->AddToCache(LocalUserNum, UserInfo);
                }
                Delegate.ExecuteIfBound(true, *UserIdShared, QueryOptions, ExternalIds, TEXT(""));
            }));
    return true;
}

void FOnlineUserInterfaceEOS::GetExternalIdMappings(
    const FExternalIdQueryOptions &QueryOptions,
    const TArray<FString> &ExternalIds,
    TArray<TSharedPtr<const FUniqueNetId>> &OutIds)
{
    if (QueryOptions.bLookupByDisplayName)
    {
        UE_LOG(LogOnlineSubsystemRedpointEOS, Error, TEXT("bLookupByDisplayName must be false"));
        return;
    }

    OutIds = TArray<TSharedPtr<const FUniqueNetId>>();
    for (int i = 0; i < ExternalIds.Num(); i++)
    {
        TSharedPtr<const FUniqueNetId> UserId = nullptr;
        for (const auto &KV : this->Cache)
        {
            if (!UserId.IsValid())
            {
                auto OnlineUserEOS = this->GetFromCacheByExternalAccountId(
                    KV.Key,
                    ExternalAccountTypeMap.Get(QueryOptions.AuthType),
                    ExternalIds[i]);
                if (OnlineUserEOS.IsValid())
                {
                    UserId = OnlineUserEOS->GetUserId();
                }
            }
            if (UserId.IsValid())
            {
                break;
            }
        }
        OutIds.Add(UserId);
    }
}

TSharedPtr<const FUniqueNetId> FOnlineUserInterfaceEOS::GetExternalIdMapping(
    const FExternalIdQueryOptions &QueryOptions,
    const FString &ExternalId)
{
    if (QueryOptions.bLookupByDisplayName)
    {
        UE_LOG(LogOnlineSubsystemRedpointEOS, Error, TEXT("bLookupByDisplayName must be false"));
        return nullptr;
    }

    TSharedPtr<const FUniqueNetId> UserId = nullptr;
    for (const auto &KV : this->Cache)
    {
        if (!UserId.IsValid())
        {
            auto OnlineUserEOS = this->GetFromCacheByExternalAccountId(
                KV.Key,
                ExternalAccountTypeMap.Get(QueryOptions.AuthType),
                ExternalId);
            if (OnlineUserEOS.IsValid())
            {
                UserId = OnlineUserEOS->GetUserId();
            }
        }
        if (UserId.IsValid())
        {
            break;
        }
    }

    return UserId;
}

}

REDPOINT_EOS_CODE_GUARD_END()