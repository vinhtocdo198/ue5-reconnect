// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSAuth/AuthenticationGraphRefreshEOSCredentials.h"
#include "RedpointEOSAuth/CrossPlatform/CrossPlatformAccountId.h"
#include "RedpointEOSAuth/OnlineExternalCredentials.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSIdentity/IdentityProcessGlobalUserHandle.h"
#include "RedpointEOSIdentity/IdentityUserId.h"
#include "RedpointEOSIdentity/IdentityUserSlot.h"
#include "RedpointEOSUserCache/UserInfo.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(526824123, Redpoint::EOS::Identity, IIdentitySystem)

namespace REDPOINT_EOS_FILE_NS_ID(2973680834, Redpoint::EOS::Identity)
{
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Auth;
using namespace ::Redpoint::EOS::Auth::CrossPlatform;
using namespace ::Redpoint::EOS::UserCache;

/**
 * Represents the state and authentication information of a locally signed in user.
 */
class REDPOINTEOSIDENTITY_API FIdentityUser : public TSharedFromThis<FIdentityUser>
{
private:
    FPlatformHandle PlatformHandle;
    FIdentityUserSlot UserSlot;
    FIdentityUserId UserId;
    // @todo Remove as this is OSSv1 specific.
    FName NativeSubsystemName;
    FAuthenticationGraphRefreshEOSCredentials RefreshCallback;
    TSharedPtr<IOnlineExternalCredentials> ExternalCredentials;
    TSharedPtr<const FCrossPlatformAccountId> CrossPlatformAccountId;
    FIdentityProcessGlobalUserHandle ProcessGlobalUserHandle;
    TMap<FString, FString> AuthenticationAttributes;
    FUserInfoPtr UserInfo;

    FIdentityUser(
        const FPlatformHandle &InPlatformHandle,
        const FIdentityUserSlot &InUserSlot,
        const FIdentityUserId &InUserId,
        const FName &InNativeSubsystemName,
        const FAuthenticationGraphRefreshEOSCredentials &InRefreshCallback,
        const TSharedPtr<IOnlineExternalCredentials> &InExternalCredentials,
        const TSharedPtr<const FCrossPlatformAccountId> &InCrossPlatformAccountId,
        const TMap<FString, FString> &InAuthenticationAttributes,
        const FUserInfoPtr &InUserInfo);
    void SetProcessGlobalUserHandle(const FIdentityProcessGlobalUserHandle &InProcessGlobalUserHandle);

public:
    static TSharedRef<FIdentityUser> Create(
        const TSharedRef<IIdentitySystem> &InIdentitySystem,
        const FPlatformHandle &InPlatformHandle,
        const FIdentityUserSlot &InUserSlot,
        const FIdentityUserId &InUserId,
        const FName &InNativeSubsystemName,
        const FAuthenticationGraphRefreshEOSCredentials &InRefreshCallback,
        const TSharedPtr<IOnlineExternalCredentials> &InExternalCredentials,
        const TSharedPtr<const FCrossPlatformAccountId> &InCrossPlatformAccountId,
        const TMap<FString, FString> &InAuthenticationAttributes,
        const FUserInfoPtr &InUserInfo);
    UE_NONCOPYABLE(FIdentityUser);
    virtual ~FIdentityUser();

    FIdentityUserSlot GetUserSlot() const;
    FIdentityUserId GetUserId() const;
    FName GetNativeSubsystemName() const;
    FAuthenticationGraphRefreshEOSCredentials GetRefreshCallback() const;
    TSharedPtr<IOnlineExternalCredentials> GetExternalCredentials() const;
    TSharedPtr<const FCrossPlatformAccountId> GetCrossPlatformAccountId() const;
    FIdentityProcessGlobalUserHandle GetProcessGlobalUserHandle() const;
    TSet<FString> GetAuthenticationAttributeKeys() const;
    bool TryGetAuthenticationAttributeValue(const FString &InKey, FString &OutValue) const;
    TMap<FString, FString> &GetStaticAuthenticationAttributes();
    FString GetIdToken() const;
    FUserInfoPtr GetUserInfo() const;
};

typedef TSharedRef<FIdentityUser> FIdentityUserRef;
typedef TSharedPtr<FIdentityUser> FIdentityUserPtr;

}

namespace Redpoint::EOS::Identity
{
REDPOINT_EOS_FILE_NS_EXPORT(2973680834, Redpoint::EOS::Identity, FIdentityUser)
REDPOINT_EOS_FILE_NS_EXPORT(2973680834, Redpoint::EOS::Identity, FIdentityUserRef)
REDPOINT_EOS_FILE_NS_EXPORT(2973680834, Redpoint::EOS::Identity, FIdentityUserPtr)
}

REDPOINT_EOS_CODE_GUARD_END()