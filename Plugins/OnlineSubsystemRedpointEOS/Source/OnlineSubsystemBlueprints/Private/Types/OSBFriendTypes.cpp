// Copyright June Rhodes. All Rights Reserved.

#include "Types/OSBFriendTypes.h"

#include "Interfaces/OnlinePresenceInterface.h"

UOnlineFriendRef *UOnlineFriendRef::FromFriend(const TSharedPtr<FOnlineFriend> &Friend)
{
    auto Ref = NewObject<UOnlineFriendRef>();
    Ref->Friend = Friend;
    Ref->AssignFromUser(Friend);
    return Ref;
}

EInviteStatus_ UOnlineFriendRef::GetInviteStatus_Implementation()
{
    return this->GetInviteStatus_Native();
}

FOnlineUserPresenceData UOnlineFriendRef::GetPresence_Implementation()
{
    return this->GetPresence_Native();
}

EInviteStatus_ UOnlineFriendRef::GetInviteStatus_Native()
{
    if (!this->Friend.IsValid())
    {
        return EInviteStatus_::Unknown;
    }
    return (EInviteStatus_)this->Friend->GetInviteStatus();
}

FOnlineUserPresenceData UOnlineFriendRef::GetPresence_Native()
{
    if (!this->Friend.IsValid())
    {
        return FOnlineUserPresenceData();
    }

    if (this->Friend->GetUserId()->GetType() == STEAM_SUBSYSTEM)
    {
        // Steam is broken and returns invalid references from GetPresence (it points to a TSharedPtr on the stack frame
        // of GetPresence, which gets released making the reference invalid). Workaround it by querying the
        // IOnlinePresence interface of Steam directly.
        auto *OSSv1 = IOnlineSubsystem::Get(STEAM_SUBSYSTEM);
        if (OSSv1 != nullptr)
        {
            auto OnlinePresence = OSSv1->GetPresenceInterface();
            if (OnlinePresence.IsValid())
            {
                TSharedPtr<FOnlineUserPresence> PresenceStatus;
                if (OnlinePresence->GetCachedPresence(*this->Friend->GetUserId(), PresenceStatus) ==
                        EOnlineCachedResult::Success &&
                    PresenceStatus.IsValid())
                {
                    return FOnlineUserPresenceData::FromNative(*PresenceStatus);
                }
            }
        }

        return FOnlineUserPresenceData();
    }

    const FOnlineUserPresence &Presence = this->Friend->GetPresence();
    return FOnlineUserPresenceData::FromNative(Presence);
}

FReportPlayedWithUserInfo FReportPlayedWithUserInfo::FromNative(const FReportPlayedWithUser &InObj)
{
    FReportPlayedWithUserInfo Result;
    Result.UserId = FUniqueNetIdRepl(InObj.UserId);
    Result.PresenceStr = InObj.PresenceStr;
    return Result;
}

FReportPlayedWithUser FReportPlayedWithUserInfo::ToNative() const
{
    FReportPlayedWithUser Result(this->UserId.GetUniqueNetId().ToSharedRef(), this->PresenceStr);
    return Result;
}

FFriendSettingsData FFriendSettingsData::FromNative(const FFriendSettings &InObj)
{
    FFriendSettingsData Result;
    Result.Data = InObj.SettingsMap;
    return Result;
}

FFriendSettings FFriendSettingsData::ToNative() const
{
    FFriendSettings Result;
    Result.SettingsMap = this->Data;
    return Result;
}

FOnlineFriendSettingsSourceDataConfig FOnlineFriendSettingsSourceDataConfig::FromNative(
    const TSharedRef<FOnlineFriendSettingsSourceData> &InObj)
{
    FOnlineFriendSettingsSourceDataConfig Result;
    Result.NeverShowAgain = InObj->bNeverShowAgain;
    return Result;
}

FOnlineFriendSettingsSourceData FOnlineFriendSettingsSourceDataConfig::ToNative() const
{
    FOnlineFriendSettingsSourceData Result;
    Result.bNeverShowAgain = this->NeverShowAgain;
    return Result;
}