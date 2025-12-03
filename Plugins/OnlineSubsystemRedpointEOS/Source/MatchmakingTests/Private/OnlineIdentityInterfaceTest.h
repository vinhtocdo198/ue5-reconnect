// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "Interfaces/OnlineIdentityInterface.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

class FOnlineIdentityInterfaceTest : public IOnlineIdentity,
                                     public TSharedFromThis<FOnlineIdentityInterfaceTest, ESPMode::ThreadSafe>
{
public:
    virtual bool Login(int32 LocalUserNum, const FOnlineAccountCredentials &AccountCredentials) override
        PURE_VIRTUAL(FOnlineIdentityInterfaceTest::Login, return false;);
    virtual bool Logout(int32 LocalUserNum) override PURE_VIRTUAL(FOnlineIdentityInterfaceTest::Logout, return false;);

    virtual bool AutoLogin(int32 LocalUserNum) override
        PURE_VIRTUAL(FOnlineIdentityInterfaceTest::AutoLogin, return false;);
    virtual TSharedPtr<FUserOnlineAccount> GetUserAccount(const FUniqueNetId &UserId) const override
        PURE_VIRTUAL(FOnlineIdentityInterfaceTest::Login, return nullptr;);
    virtual TArray<TSharedPtr<FUserOnlineAccount>> GetAllUserAccounts() const override
        PURE_VIRTUAL(FOnlineIdentityInterfaceTest::Login, return TArray<TSharedPtr<FUserOnlineAccount>>(););
    virtual TSharedPtr<const FUniqueNetId> GetUniquePlayerId(int32 LocalUserNum) const override
        PURE_VIRTUAL(FOnlineIdentityInterfaceTest::Login, return nullptr;);
    virtual TSharedPtr<const FUniqueNetId> CreateUniquePlayerId(uint8 *Bytes, int32 Size) override;
    virtual TSharedPtr<const FUniqueNetId> CreateUniquePlayerId(const FString &Str) override
        PURE_VIRTUAL(FOnlineIdentityInterfaceTest::CreateUniquePlayerId, return nullptr;);
    virtual ELoginStatus::Type GetLoginStatus(int32 LocalUserNum) const override
        PURE_VIRTUAL(FOnlineIdentityInterfaceTest::GetLoginStatus, return ELoginStatus::NotLoggedIn;);
    virtual ELoginStatus::Type GetLoginStatus(const FUniqueNetId &UserId) const override
        PURE_VIRTUAL(FOnlineIdentityInterfaceTest::GetLoginStatus, return ELoginStatus::NotLoggedIn;);
    virtual FString GetPlayerNickname(int32 LocalUserNum) const override
        PURE_VIRTUAL(FOnlineIdentityInterfaceTest::GetPlayerNickname, return TEXT(""););
    virtual FString GetPlayerNickname(const FUniqueNetId &) const override
    {
        return TEXT("");
    };
    virtual FString GetAuthToken(int32 LocalUserNum) const override
        PURE_VIRTUAL(FOnlineIdentityInterfaceTest::GetAuthToken, return TEXT(""););
    virtual void RevokeAuthToken(const FUniqueNetId &LocalUserId, const FOnRevokeAuthTokenCompleteDelegate &Delegate)
        override PURE_VIRTUAL(FOnlineIdentityInterfaceTest::RevokeAuthToken, return;);
#if REDPOINT_EOS_UE_5_4_OR_LATER
    virtual void GetUserPrivilege(
        const FUniqueNetId &LocalUserId,
        EUserPrivileges::Type Privilege,
        const FOnGetUserPrivilegeCompleteDelegate &Delegate,
        EShowPrivilegeResolveUI ShowResolveUI = EShowPrivilegeResolveUI::Default) override
        PURE_VIRTUAL(FOnlineIdentityInterfaceTest::GetUserPrivilege, return;);
#else
    virtual void GetUserPrivilege(
        const FUniqueNetId &LocalUserId,
        EUserPrivileges::Type Privilege,
        const FOnGetUserPrivilegeCompleteDelegate &Delegate) override
        PURE_VIRTUAL(FOnlineIdentityInterfaceTest::GetUserPrivilege, return;);
#endif
    PRAGMA_DISABLE_DEPRECATION_WARNINGS
    virtual FPlatformUserId GetPlatformUserIdFromUniqueNetId(const FUniqueNetId &UniqueNetId) const override
        PURE_VIRTUAL(FOnlineIdentityInterfaceTest::GetPlatformUserIdFromUniqueNetId,
                     return FPlatformUserId::CreateFromInternalId(0););
    PRAGMA_ENABLE_DEPRECATION_WARNINGS
    virtual FString GetAuthType() const override
        PURE_VIRTUAL(FOnlineIdentityInterfaceTest::GetAuthType, return TEXT(""););
};

#endif

REDPOINT_EOS_CODE_GUARD_END()