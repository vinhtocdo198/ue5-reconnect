// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Identity/OnlineIdentityInterfaceEOS.h"
#include "OnlineSubsystemRedpointEOS/VoiceChat/EOSVoiceManager.h"
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSAuth/CrossPlatform/CrossPlatformAccountId.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSConfig/Config.h"
#include "RedpointEOSCore/Utils/HasEventRegistration.h"
#include "RedpointEOSCore/Utils/IndexedTable.h"
#include "RedpointEOSIdentity/IdentitySystem.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2650835609, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Identity)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::Core::Utils;
using namespace ::Redpoint::EOS::Config;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Identity;
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat;
#endif
using namespace ::Redpoint::EOS::Auth::CrossPlatform;

class ONLINESUBSYSTEMREDPOINTEOS_API FOnlineIdentityInterfaceEOSImpl
    : public IOnlineIdentityInterfaceEOS,
      public TSharedFromThis<FOnlineIdentityInterfaceEOSImpl, ESPMode::ThreadSafe>,
      public IHasEventRegistration
{
private:
    FPlatformHandle PlatformHandle;
    FName InstanceName;
    TSharedRef<IConfig> Config;
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    TSharedPtr<FOnlineSubsystemRedpointEAS> EpicGamesOnlineSubsystem;
#endif
    TSharedRef<IIdentitySystem> IdentitySystem;
    FDelegateHandle OnUserSlotChangedHandle;
    FIndexedTable LocalAccounts;

public:
    FOnlineIdentityInterfaceEOSImpl(
        const FPlatformHandle &InPlatformHandle,
        const FName &InInstanceName,
        const TSharedRef<IConfig> &InConfig
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
        ,
        const TSharedPtr<FOnlineSubsystemRedpointEAS> &InEpicGamesOnlineSubsystem
#endif
    );
    virtual void RegisterEvents() override;
    UE_NONCOPYABLE(FOnlineIdentityInterfaceEOSImpl);
    virtual ~FOnlineIdentityInterfaceEOSImpl() override;

    virtual bool Login(int32 LocalUserNum, const FOnlineAccountCredentials &AccountCredentials) override;
    virtual bool Logout(int32 LocalUserNum) override;
    virtual bool AutoLogin(int32 LocalUserNum) override;

private:
    FUniqueNetIdRef ConvertToUniqueNetId(FIdentityUserId UserId) const;

    void OnUserSlotChanged(
        FIdentityUserSlot UserSlot,
        EIdentityUserSlotState OldUserSlotState,
        FIdentityUserId OldUserId,
        EIdentityUserSlotState NewUserSlotState,
        FIdentityUserId NewUserId,
        EIdentityUserSlotChangeReason ChangeReason);
    void OnLoginComplete(FError ErrorCode, FIdentityUserPtr NewUser, int LocalUserNum);
    void OnLogoutComplete(FError ErrorCode, int LocalUserNum);

public:
    virtual TSharedPtr<FUserOnlineAccount> GetUserAccount(const FUniqueNetId &UserId) const override;
    virtual TArray<TSharedPtr<FUserOnlineAccount>> GetAllUserAccounts() const override;
    virtual FUniqueNetIdPtr GetUniquePlayerId(int32 LocalUserNum) const override;
    virtual FUniqueNetIdPtr CreateUniquePlayerId(uint8 *Bytes, int32 Size) override;
    virtual FUniqueNetIdPtr CreateUniquePlayerId(const FString &Str) override;
    virtual ELoginStatus::Type GetLoginStatus(int32 LocalUserNum) const override;
    virtual ELoginStatus::Type GetLoginStatus(const FUniqueNetId &UserId) const override;
    virtual FString GetPlayerNickname(int32 LocalUserNum) const override;
    virtual FString GetPlayerNickname(const FUniqueNetId &UserId) const override;
    virtual FString GetAuthToken(int32 LocalUserNum) const override;
    virtual void RevokeAuthToken(const FUniqueNetId &LocalUserId, const FOnRevokeAuthTokenCompleteDelegate &Delegate)
        override;
    virtual void ClearCachedAuthToken(const FUniqueNetId &UserId) override;
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
    virtual FPlatformUserId GetPlatformUserIdFromLocalUserNum(int32 LocalUserNum) const override;
    virtual int32 GetLocalUserNumFromPlatformUserId(FPlatformUserId PlatformUserId) const override;
    virtual FString GetAuthType() const override;

    virtual bool GetLocalUserNum(const FUniqueNetId &UniqueNetId, int32 &OutLocalUserNum) const override;
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    virtual TSharedPtr<const FCrossPlatformAccountId> GetCrossPlatformAccountId(
        const FUniqueNetId &UniqueNetId) const override;
    virtual bool IsCrossPlatformAccountProviderAvailable() const override;
#endif

protected:
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    virtual TSharedPtr<FOnlineSubsystemRedpointEAS> GetEpicGamesOnlineSubsystem() const override;
#endif
    virtual FName GetInstanceName() const override;
    virtual bool GetNativeSubsystemName(int32 LocalUserNum, FName &OutNativeSubsystemName) const override;
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Identity
{
REDPOINT_EOS_FILE_NS_EXPORT(
    2650835609,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Identity,
    FOnlineIdentityInterfaceEOSImpl)
}

REDPOINT_EOS_CODE_GUARD_END()