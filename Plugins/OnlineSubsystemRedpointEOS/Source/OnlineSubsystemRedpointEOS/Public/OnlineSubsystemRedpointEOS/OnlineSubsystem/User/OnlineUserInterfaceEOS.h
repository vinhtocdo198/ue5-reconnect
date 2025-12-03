// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Interfaces/OnlineUserInterface.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Identity/OnlineIdentityInterfaceEOS.h"
#include "OnlineSubsystemRedpointEOS/User/OnlineUserEOS.h"
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Id/UniqueNetIdEOS.h"
#include "RedpointEOSUserCache/UserInfo.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(757011916, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::User)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Identity;
using namespace ::Redpoint::EOS::UserCache;

class ONLINESUBSYSTEMREDPOINTEOS_API FOnlineUserInterfaceEOS : public IOnlineUser,
                                                               public TSharedFromThis<FOnlineUserInterfaceEOS>
{
private:
    class FPerUserCache
    {
    public:
        TUserIdMap<TSharedPtr<FOnlineUserEOS>> UsersById;
        TMap<EOS_EExternalAccountType, TMap<FString, TSharedPtr<FOnlineUserEOS>>> UsersByExternalId;
        TMap<FString, TSharedPtr<FOnlineUserEOS>> UsersByEpicGamesDisplayName;
    };

    TSharedRef<IOnlineIdentityInterfaceEOS> Identity;
    FPlatformHandle PlatformHandle;
    TMap<int32, FPerUserCache> Cache;

    void AddToCache(int32 LocalUserNum, const FUserInfoRef &UserInfo);
    TSharedPtr<FOnlineUserEOS> GetFromCache(int32 LocalUserNum, const FUniqueNetId &UserId) const;
    TSharedPtr<FOnlineUserEOS> GetFromCacheByExternalAccountId(
        int32 LocalUserNum,
        EOS_EExternalAccountType ExternalAccountType,
        const FString &ExternalAccountId) const;
    TSharedPtr<FOnlineUserEOS> GetFromCacheByEpicGamesDisplayName(
        int32 LocalUserNum,
        const FString &EpicGamesDisplayName) const;

public:
    FOnlineUserInterfaceEOS(
        const TSharedRef<IOnlineIdentityInterfaceEOS> &InIdentity,
        const FPlatformHandle &InPlatformHandle);
    UE_NONCOPYABLE(FOnlineUserInterfaceEOS);
    virtual ~FOnlineUserInterfaceEOS() override = default;

    virtual bool QueryUserInfo(int32 LocalUserNum, const TArray<TSharedRef<const FUniqueNetId>> &UserIds) override;

    virtual bool GetAllUserInfo(int32 LocalUserNum, TArray<TSharedRef<FOnlineUser>> &OutUsers) override;

    virtual TSharedPtr<FOnlineUser> GetUserInfo(int32 LocalUserNum, const FUniqueNetId &UserId) override;

    virtual bool QueryUserIdMapping(
        const FUniqueNetId &UserId,
        const FString &DisplayNameOrEmail,
        const FOnQueryUserMappingComplete &Delegate = FOnQueryUserMappingComplete()) override;

    virtual bool QueryExternalIdMappings(
        const FUniqueNetId &UserId,
        const FExternalIdQueryOptions &QueryOptions,
        const TArray<FString> &ExternalIds,
        const FOnQueryExternalIdMappingsComplete &Delegate = FOnQueryExternalIdMappingsComplete()) override;

    virtual void GetExternalIdMappings(
        const FExternalIdQueryOptions &QueryOptions,
        const TArray<FString> &ExternalIds,
        TArray<TSharedPtr<const FUniqueNetId>> &OutIds) override;

    virtual TSharedPtr<const FUniqueNetId> GetExternalIdMapping(
        const FExternalIdQueryOptions &QueryOptions,
        const FString &ExternalId) override;
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::User
{
REDPOINT_EOS_FILE_NS_EXPORT(
    757011916,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::User,
    FOnlineUserInterfaceEOS)
}

REDPOINT_EOS_CODE_GUARD_END()