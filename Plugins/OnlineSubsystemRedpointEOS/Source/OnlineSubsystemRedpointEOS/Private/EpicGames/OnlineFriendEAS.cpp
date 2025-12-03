// Copyright June Rhodes. All Rights Reserved.

#if EOS_HAS_AUTHENTICATION

#include "OnlineSubsystemRedpointEOS/Shared/EpicGames/OnlineFriendEAS.h"

#include "OnlineSubsystemRedpointEOS/Shared/EpicGames/UniqueNetIdEAS.h"
#include "RedpointEOSCore/Legacy/EOSString.h"

EOS_ENABLE_STRICT_WARNINGS

FOnlineFriendEAS::FOnlineFriendEAS(
    TSharedRef<const FUniqueNetIdEAS> InUserId,
    TSharedRef<const FOnlineUserPresenceEAS> InPresenceInfo,
    EOS_UserInfo *InUserInfo)
    : UserId(MoveTemp(InUserId))
    , PresenceInfo(MoveTemp(InPresenceInfo))
    , UserInfo(InUserInfo)
{
}

FOnlineFriendEAS::~FOnlineFriendEAS()
{
    EOS_UserInfo_Release(this->UserInfo);
    this->UserInfo = nullptr;
}

TSharedRef<const FUniqueNetId> FOnlineFriendEAS::GetUserId() const
{
    return this->UserId;
}

FString FOnlineFriendEAS::GetRealName() const
{
    if (this->UserInfo->Nickname != nullptr)
    {
        return EOSString_UserInfo_DisplayName::FromUtf8String(this->UserInfo->Nickname);
    }

    return EOSString_UserInfo_DisplayName::FromUtf8String(this->UserInfo->DisplayNameSanitized);
}

FString FOnlineFriendEAS::GetDisplayName(const FString &Platform) const
{
    return this->GetRealName();
}

bool FOnlineFriendEAS::GetUserAttribute(const FString &AttrName, FString &OutAttrValue) const
{
    if (AttrName == TEXT("country"))
    {
        OutAttrValue = UTF8_TO_TCHAR(this->UserInfo->Country);
        return true;
    }
    else if (AttrName == TEXT("preferredLanguage"))
    {
        OutAttrValue = UTF8_TO_TCHAR(this->UserInfo->PreferredLanguage);
        return true;
    }

    return false;
}

EInviteStatus::Type FOnlineFriendEAS::GetInviteStatus() const
{
    return EInviteStatus::Accepted;
}

const class FOnlineUserPresence &FOnlineFriendEAS::GetPresence() const
{
    return *this->PresenceInfo;
}

EOS_DISABLE_STRICT_WARNINGS

#endif // #if EOS_HAS_AUTHENTICATION