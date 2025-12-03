// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSIdentity/IdentityUser.h"

#include "RedpointEOSAPI/Auth/CopyIdToken.h"
#include "RedpointEOSAPI/Connect/CopyIdToken.h"
#include "RedpointEOSConfig/Config.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSIdentity/IdentityHookRegistry.h"
#include "RedpointEOSIdentity/IdentityProcessGlobalUserRegistry.h"
#include "RedpointEOSIdentity/Logging.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2973680834, Redpoint::EOS::Identity)
{
using namespace ::Redpoint::EOS::Identity;

FIdentityUser::FIdentityUser(
    const FPlatformHandle &InPlatformHandle,
    const FIdentityUserSlot &InUserSlot,
    const FIdentityUserId &InUserId,
    const FName &InNativeSubsystemName,
    const FAuthenticationGraphRefreshEOSCredentials &InRefreshCallback,
    const TSharedPtr<IOnlineExternalCredentials> &InExternalCredentials,
    const TSharedPtr<const FCrossPlatformAccountId> &InCrossPlatformAccountId,
    const TMap<FString, FString> &InAuthenticationAttributes,
    const FUserInfoPtr &InUserInfo)
    : PlatformHandle(InPlatformHandle)
    , UserSlot(InUserSlot)
    , UserId(InUserId)
    , NativeSubsystemName(InNativeSubsystemName)
    , RefreshCallback(InRefreshCallback)
    , ExternalCredentials(InExternalCredentials)
    , CrossPlatformAccountId(InCrossPlatformAccountId)
    , ProcessGlobalUserHandle()
    , AuthenticationAttributes(InAuthenticationAttributes)
    , UserInfo(InUserInfo)
{
    using namespace Redpoint::EOS::Core::Id;
    checkf(
        IsDedicatedServerId(this->UserId) || this->UserInfo.IsValid(),
        TEXT("InUserInfo can only be nullptr if this FIdentityUser is for a dedicated server."));
}

void FIdentityUser::SetProcessGlobalUserHandle(const FIdentityProcessGlobalUserHandle &InProcessGlobalUserHandle)
{
    this->ProcessGlobalUserHandle = InProcessGlobalUserHandle;
}

TSharedRef<FIdentityUser> FIdentityUser::Create(
    const TSharedRef<IIdentitySystem> &InIdentitySystem,
    const FPlatformHandle &InPlatformHandle,
    const FIdentityUserSlot &InUserSlot,
    const FIdentityUserId &InUserId,
    const FName &InNativeSubsystemName,
    const FAuthenticationGraphRefreshEOSCredentials &InRefreshCallback,
    const TSharedPtr<IOnlineExternalCredentials> &InExternalCredentials,
    const TSharedPtr<const FCrossPlatformAccountId> &InCrossPlatformAccountId,
    const TMap<FString, FString> &InAuthenticationAttributes,
    const FUserInfoPtr &InUserInfo)
{
    TSharedRef<FIdentityUser> User = MakeShareable(new FIdentityUser(
        InPlatformHandle,
        InUserSlot,
        InUserId,
        InNativeSubsystemName,
        InRefreshCallback,
        InExternalCredentials,
        InCrossPlatformAccountId,
        InAuthenticationAttributes,
        InUserInfo));
    User->SetProcessGlobalUserHandle(FIdentityProcessGlobalUserRegistry::Register(InIdentitySystem, User));
    return User;
}

FIdentityUser::~FIdentityUser()
{
    FIdentityProcessGlobalUserRegistry::Unregister(this->ProcessGlobalUserHandle);
}

FIdentityUserSlot FIdentityUser::GetUserSlot() const
{
    return this->UserSlot;
}

FIdentityUserId FIdentityUser::GetUserId() const
{
    return this->UserId;
}

FName FIdentityUser::GetNativeSubsystemName() const
{
    return this->NativeSubsystemName;
}

FAuthenticationGraphRefreshEOSCredentials FIdentityUser::GetRefreshCallback() const
{
    return this->RefreshCallback;
}

TSharedPtr<IOnlineExternalCredentials> FIdentityUser::GetExternalCredentials() const
{
    return this->ExternalCredentials;
}

TSharedPtr<const FCrossPlatformAccountId> FIdentityUser::GetCrossPlatformAccountId() const
{
    return this->CrossPlatformAccountId;
}

FIdentityProcessGlobalUserHandle FIdentityUser::GetProcessGlobalUserHandle() const
{
    return this->ProcessGlobalUserHandle;
}

TSet<FString> FIdentityUser::GetAuthenticationAttributeKeys() const
{
    TSet<FString> Keys;
    for (const auto &KV : this->AuthenticationAttributes)
    {
        Keys.Add(KV.Key);
    }
    Keys.Add(TEXT("crossPlatform.canLink"));
    Keys.Add(TEXT("idToken"));
    if (this->CrossPlatformAccountId.IsValid() &&
        this->CrossPlatformAccountId->GetType().IsEqual(EPIC_GAMES_ACCOUNT_ID))
    {
        Keys.Add(TEXT("epic.idToken"));
    }
    FIdentityHookContext Context(this->PlatformHandle->GetSystem<IIdentitySystem>(), this->PlatformHandle);
    for (const auto &Hook : FIdentityHookRegistry::Get())
    {
        Hook->OnGetAdditionalAuthenticationAttributeKeys(Context, this->AsShared(), Keys);
    }
    return Keys;
}

bool FIdentityUser::TryGetAuthenticationAttributeValue(const FString &InKey, FString &OutValue) const
{
    if (InKey == TEXT("crossPlatform.canLink"))
    {
        FName AccountProvider = this->PlatformHandle->GetEnvironment()->Config->GetCrossPlatformAccountProvider();

        /**
         * If CrossPlatformAccountId is valid then we already have a cross platform account linked. If AccountProvider
         * is none, we don't have a cross platform provider to link to, and if ESupportedLinkType is none then the
         * credentials do not support linking with the cross platform account provider.
         */

        auto CanLink =
            !this->CrossPlatformAccountId.IsValid() && !AccountProvider.IsNone() &&
            this->ExternalCredentials->GetProviderSupportedLinkType(AccountProvider) != ESupportedLinkType::None;

        OutValue = CanLink ? TEXT("true") : TEXT("false");
        return true;
    }

    if (InKey == TEXT("idToken"))
    {
        OutValue = this->GetIdToken();
        return !OutValue.IsEmpty();
    }

    if (InKey == TEXT("epic.idToken") && this->CrossPlatformAccountId.IsValid() &&
        this->CrossPlatformAccountId->GetType().IsEqual(EPIC_GAMES_ACCOUNT_ID))
    {
        using namespace Redpoint::EOS::Core::Id;
        using namespace Redpoint::EOS::API::Auth;

        EOS_EResult ResultCode;
        FCopyIdToken::Result Result;
        FCopyIdToken::Execute(
            this->PlatformHandle,
            FCopyIdToken::Options{GetEpicGamesAccountId(this->CrossPlatformAccountId->ToString())},
            ResultCode,
            Result);
        if (ResultCode == EOS_EResult::EOS_Success)
        {
            OutValue = Result.JsonWebToken;
            return true;
        }
        else
        {
            UE_LOG(
                LogRedpointEOSIdentity,
                Warning,
                TEXT("Unable to obtain EOS Auth ID token for account: %s"),
                *Redpoint::EOS::API::ConvertError(
                     ANSI_TO_TCHAR(__FUNCTION__),
                     TEXT("Attempt to obtain EOS Auth ID token for account failed"),
                     ResultCode)
                     .ToLogString());
            return false;
        }
    }

    auto *AttributeValue = this->AuthenticationAttributes.Find(InKey);
    if (AttributeValue != nullptr)
    {
        OutValue = *AttributeValue;
        return true;
    }

    FIdentityHookContext Context(this->PlatformHandle->GetSystem<IIdentitySystem>(), this->PlatformHandle);
    for (const auto &Hook : FIdentityHookRegistry::Get())
    {
        if (Hook->OnGetAdditionalAuthenticationAttributeValue(Context, this->AsShared(), InKey, OutValue))
        {
            return true;
        }
    }

    return false;
}

TMap<FString, FString> &FIdentityUser::GetStaticAuthenticationAttributes()
{
    return this->AuthenticationAttributes;
}

FString FIdentityUser::GetIdToken() const
{
    using namespace Redpoint::EOS::API::Connect;
    using namespace Redpoint::EOS::Core::Id;

    EOS_EResult ResultCode;
    FCopyIdToken::Result Result;
    FCopyIdToken::Execute(
        this->PlatformHandle,
        FCopyIdToken::Options{GetProductUserId(this->UserId)},
        ResultCode,
        Result);
    if (ResultCode == EOS_EResult::EOS_Success)
    {
        return Result.JsonWebToken;
    }
    else
    {
        UE_LOG(
            LogRedpointEOSIdentity,
            Warning,
            TEXT("Unable to obtain EOS Connect ID token for account: %s"),
            *Redpoint::EOS::API::ConvertError(
                 ANSI_TO_TCHAR(__FUNCTION__),
                 TEXT("Attempt to obtain EOS Connect ID token for account failed."),
                 ResultCode)
                 .ToLogString());
        return TEXT("");
    }
}

FUserInfoPtr FIdentityUser::GetUserInfo() const
{
    return this->UserInfo;
}

}

REDPOINT_EOS_CODE_GUARD_END()