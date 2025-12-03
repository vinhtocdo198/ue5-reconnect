// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Social/OnlineFriendNonUnifiedEOS.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "RedpointEOSFriends/FriendSystem.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1131179970, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Social)
{
using namespace ::Redpoint::EOS::Friends;

FOnlineFriendNonUnifiedEOS::FOnlineFriendNonUnifiedEOS(
    const FUniqueNetIdRef &InUniqueNetId,
    const FPlatformHandle &InPlatformHandle,
    const FIdentityUserId &InLocalUserId,
    const FExternalUserInfoRef &InFriend)
    : UniqueNetId(InUniqueNetId)
    , PlatformHandle(InPlatformHandle)
    , LocalUserId(InLocalUserId)
    , Friend(InFriend)
    , PresenceState()
    , InviteState(EInviteStatus::Unknown)
{
    this->PresenceState.Status.State = EOnlinePresenceState::Offline;
}

FUniqueNetIdRef FOnlineFriendNonUnifiedEOS::GetUserId() const
{
    return this->UniqueNetId;
}

FString FOnlineFriendNonUnifiedEOS::GetRealName() const
{
    return this->GetDisplayName(TEXT(""));
}

FString FOnlineFriendNonUnifiedEOS::GetDisplayName(const FString &Platform) const
{
    for (const auto &KV : this->Friend->GetLinkedExternalAccounts())
    {
        return KV.Value.DisplayName;
    }

    return TEXT("");
}

bool FOnlineFriendNonUnifiedEOS::GetUserAttribute(const FString &AttrName, FString &OutAttrValue) const
{
    auto *AttrValue = this->Friend->GetAttributes().Find(AttrName);
    if (AttrValue != nullptr)
    {
        OutAttrValue = *AttrValue;
        return true;
    }
    return false;
}

bool FOnlineFriendNonUnifiedEOS::SetUserLocalAttribute(const FString &AttrName, const FString &InAttrValue)
{
    return false;
}

EInviteStatus::Type FOnlineFriendNonUnifiedEOS::GetInviteStatus() const
{
    return this->InviteState;
}

const FOnlineUserPresence &FOnlineFriendNonUnifiedEOS::GetPresence() const
{
    return this->PresenceState;
}

const FExternalUserInfoRef &FOnlineFriendNonUnifiedEOS::GetFriend() const
{
    return this->Friend;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()