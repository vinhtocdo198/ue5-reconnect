// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSFriends/Unification/UnifiedUserInfo.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "RedpointEOSAPI/Common/ExternalAccountType.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3225458625, Redpoint::EOS::Friends::Unification)
{
using namespace ::Redpoint::EOS::API::Common;

FUnifiedUserInfo::FUnifiedUserInfo(
    const TSharedRef<FUserInfo> &InWrappedUserInfo,
    const TMap<FString, TSharedPtr<FExternalUserInfo>> &InRuntimePlatformIntegrationExternalFriends,
    const TOptional<FString> &InAvatarUrl)
    : WrappedUserInfo(InWrappedUserInfo)
    , RuntimePlatformIntegrationExternalFriends(InRuntimePlatformIntegrationExternalFriends)
    , MaterializedAttributes()
    , AvatarUrl(InAvatarUrl)
{
}

bool FUnifiedUserInfo::IsReady() const
{
    return this->WrappedUserInfo->IsReady();
}

UE::Online::FAccountId FUnifiedUserInfo::GetUserId() const
{
    return this->WrappedUserInfo->GetUserId();
}

TOptional<EOS_EExternalAccountType> FUnifiedUserInfo::GetMostRecentlySignedInPlatform() const
{
    return this->WrappedUserInfo->GetMostRecentlySignedInPlatform();
}

TOptional<const FUserEpicGamesAccountInfo> FUnifiedUserInfo::GetLinkedEpicGamesAccount() const
{
    return this->WrappedUserInfo->GetLinkedEpicGamesAccount();
}

const TMap<EOS_EExternalAccountType, const FUserExternalAccountInfo> &FUnifiedUserInfo::GetLinkedExternalAccounts()
    const
{
    // I can't see any reason why this would need to aggregate RuntimePlatformIntegrationExternalFriends, since the PUID
    // should have them all linked anyway (otherwise how did we unify them?)
    return this->WrappedUserInfo->GetLinkedExternalAccounts();
}

const TMap<FString, FString> &FUnifiedUserInfo::GetAttributes() const
{
    if (this->MaterializedAttributes.IsSet())
    {
        return this->MaterializedAttributes.GetValue();
    }

    if (!this->WrappedUserInfo->IsReady())
    {
        return this->WrappedUserInfo->GetAttributes();
    }

    // Need to materalize.
    return const_cast<FUnifiedUserInfo *>(this)->MaterializeAttributes();
}

const TMap<FString, FString> &FUnifiedUserInfo::MaterializeAttributes()
{
    TMap<FString, FString> Attributes;

    for (const auto &KV : this->WrappedUserInfo->GetAttributes())
    {
        Attributes.Add(KV.Key, KV.Value);
    }

    if (this->AvatarUrl.IsSet())
    {
        Attributes.Add(TEXT("avatarUrl"), this->AvatarUrl.GetValue());
    }

    auto PlatformNames = TArray<FString>();
    for (const auto &PlatformToExternalUserInfo : this->RuntimePlatformIntegrationExternalFriends)
    {
        PlatformNames.Add(PlatformToExternalUserInfo.Key.ToLower());

        auto Prefix = FString::Printf(TEXT("runtimePlatform.friend.%s"), *PlatformToExternalUserInfo.Key.ToLower());
        for (const auto &LinkedAccountKV : PlatformToExternalUserInfo.Value->GetLinkedExternalAccounts())
        {
            Attributes.Add(FString::Printf(TEXT("%s.id"), *Prefix), LinkedAccountKV.Value.AccountId);
            Attributes.Add(FString::Printf(TEXT("%s.displayName"), *Prefix), LinkedAccountKV.Value.DisplayName);

            // deprecated
            Attributes.Add(FString::Printf(TEXT("%s.realName"), *Prefix), LinkedAccountKV.Value.DisplayName);

            break;
        }
        for (const auto &ExternalAttribute : PlatformToExternalUserInfo.Value->GetAttributes())
        {
            Attributes.Add(
                FString::Printf(TEXT("%s.attr.%s"), *Prefix, *ExternalAttribute.Key),
                ExternalAttribute.Value);
        }
    }
    Attributes.Add(TEXT("runtimePlatform.platformNames"), FString::Join(PlatformNames, TEXT(",")));

    this->MaterializedAttributes = Attributes;
    return this->MaterializedAttributes.GetValue();
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()