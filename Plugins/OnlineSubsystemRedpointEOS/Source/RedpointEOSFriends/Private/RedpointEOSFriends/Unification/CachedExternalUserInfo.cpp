// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSFriends/Unification/CachedExternalUserInfo.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(4144957489, Redpoint::EOS::Friends::Unification)
{

FCachedExternalUserInfo::FCachedExternalUserInfo(const FSerializedCachedFriend &SerializedCachedFriend)
    : LinkedExternalAccounts()
    , Attributes()
    , AvatarUrl()
{
    for (const auto &KV : SerializedCachedFriend.LinkedAccounts)
    {
        FUserExternalAccountInfo ExternalAccountInfo;
        ExternalAccountInfo.AccountId = KV.Value.AccountId;
        ExternalAccountInfo.AccountIdType = KV.Key;
        ExternalAccountInfo.DisplayName = KV.Value.DisplayName;
        ExternalAccountInfo.LastLoginTime = KV.Value.LastLoginTime;
        this->LinkedExternalAccounts.Add(KV.Key, ExternalAccountInfo);
    }
    this->AvatarUrl = SerializedCachedFriend.AccountAvatarUrl;
    if (!this->AvatarUrl.IsEmpty())
    {
        this->Attributes.Add(TEXT("avatarUrl"), this->AvatarUrl);
    }
}

const TMap<EOS_EExternalAccountType, const FUserExternalAccountInfo> &FCachedExternalUserInfo::
    GetLinkedExternalAccounts() const
{
    return this->LinkedExternalAccounts;
}

const TMap<FString, FString> &FCachedExternalUserInfo::GetAttributes() const
{
    return this->Attributes;
}

TOptional<FString> FCachedExternalUserInfo::GetAvatarUrl()
{
    if (this->AvatarUrl.IsEmpty())
    {
        return TOptional<FString>();
    }
    return this->AvatarUrl;
}

};

#endif

REDPOINT_EOS_CODE_GUARD_END()