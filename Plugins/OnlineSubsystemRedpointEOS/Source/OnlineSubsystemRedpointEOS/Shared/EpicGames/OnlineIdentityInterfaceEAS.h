// Copyright June Rhodes. All Rights Reserved.

#pragma once

#if EOS_HAS_AUTHENTICATION

#include "Interfaces/OnlineIdentityInterface.h"
#include "OnlineSubsystemRedpointEOS/Shared/EpicGames/UniqueNetIdEAS.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGamesCrossPlatformAccountId.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

EOS_ENABLE_STRICT_WARNINGS

REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(
    4052375527,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::Identity,
    FEpicGamesOSSIdentityHook)

class FOnlineIdentityInterfaceEAS : public IOnlineIdentity,
                                    public TSharedFromThis<FOnlineIdentityInterfaceEAS, ESPMode::ThreadSafe>
{
    friend Redpoint::EOS::OnlineSubsystemRedpointEOS::Identity::FEpicGamesOSSIdentityHook;

private:
    TMap<int32, TSharedPtr<const FUniqueNetIdEAS>> SignedInUsers;

    // These functions are called by FOnlineIdentityInterfaceEOS whenever a local user is signed in or out, and they are
    // using an Epic account as their cross-platform account. The interface is otherwise a read-only implementation and
    // only exists so the other implemented interfaces can use it (instead of referencing the EOS side of things
    // directly).
    void UserSignedInWithEpicId(
        int32 InLocalUserNum,
        const TSharedPtr<const Redpoint::EOS::Auth::CrossPlatform::FEpicGamesCrossPlatformAccountId>
            &InCrossPlatformAccountId);
    void UserSignedOut(int32 InLocalUserNum);

public:
    FOnlineIdentityInterfaceEAS() = default;
    UE_NONCOPYABLE(FOnlineIdentityInterfaceEAS);

    virtual bool Login(int32 LocalUserNum, const FOnlineAccountCredentials &AccountCredentials) override;
    virtual bool Logout(int32 LocalUserNum) override;
    virtual bool AutoLogin(int32 LocalUserNum) override;
    virtual TSharedPtr<FUserOnlineAccount> GetUserAccount(const FUniqueNetId &UserId) const override;
    virtual TArray<TSharedPtr<FUserOnlineAccount>> GetAllUserAccounts() const override;
    virtual TSharedPtr<const FUniqueNetId> GetUniquePlayerId(int32 LocalUserNum) const override;
    virtual bool GetLocalUserNum(const FUniqueNetId &UniqueNetId, int32 &OutLocalUserNum) const;
    virtual TSharedPtr<const FUniqueNetId> CreateUniquePlayerId(uint8 *Bytes, int32 Size) override;
    virtual TSharedPtr<const FUniqueNetId> CreateUniquePlayerId(const FString &Str) override;
    virtual ELoginStatus::Type GetLoginStatus(int32 LocalUserNum) const override;
    virtual ELoginStatus::Type GetLoginStatus(const FUniqueNetId &UserId) const override;
    virtual FString GetPlayerNickname(int32 LocalUserNum) const override;
    virtual FString GetPlayerNickname(const FUniqueNetId &UserId) const override;
    virtual FString GetAuthToken(int32 LocalUserNum) const override;
    virtual void RevokeAuthToken(const FUniqueNetId &LocalUserId, const FOnRevokeAuthTokenCompleteDelegate &Delegate)
        override;
#if REDPOINT_EOS_UE_5_4_OR_LATER
    virtual void GetUserPrivilege(
        const FUniqueNetId &LocalUserId,
        EUserPrivileges::Type Privilege,
        const FOnGetUserPrivilegeCompleteDelegate &Delegate,
        EShowPrivilegeResolveUI ShowResolveUI = EShowPrivilegeResolveUI::Default) override;
#else
    virtual void GetUserPrivilege(
        const FUniqueNetId &LocalUserId,
        EUserPrivileges::Type Privilege,
        const FOnGetUserPrivilegeCompleteDelegate &Delegate) override;
#endif
    virtual FPlatformUserId GetPlatformUserIdFromUniqueNetId(const FUniqueNetId &UniqueNetId) const override;
    virtual FString GetAuthType() const override;

    /** Returns the Epic account ID of the given local player, by their local user num. */
    virtual EOS_EpicAccountId GetEpicAccountId(int32 LocalUserNum) const;
};

EOS_DISABLE_STRICT_WARNINGS

#endif // #if EOS_HAS_AUTHENTICATION