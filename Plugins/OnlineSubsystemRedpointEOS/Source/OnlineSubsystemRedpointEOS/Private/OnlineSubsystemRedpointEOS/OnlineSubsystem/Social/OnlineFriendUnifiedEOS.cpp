// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Social/OnlineFriendUnifiedEOS.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "OnlineSubsystemRedpointEOS/Logging.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSFriends/FriendSystem.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3115511217, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Social)
{
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::Friends;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS;

FOnlineFriendUnifiedEOS::FOnlineFriendUnifiedEOS(
    const FPlatformHandle &InPlatformHandle,
    const FIdentityUserId &InLocalUserId,
    const FUserInfoRef &InFriend,
    const FString &InSource)
    : PlatformHandle(InPlatformHandle)
    , LocalUserId(InLocalUserId)
    , Friend(InFriend)
    , Source(InSource)
    , PresenceState()
    , InviteState(EInviteStatus::Unknown)
    , LastSeen(TOptional<FDateTime>())
{
}

FUniqueNetIdRef FOnlineFriendUnifiedEOS::GetUserId() const
{
    return GetUniqueNetIdRef(this->Friend->GetUserId());
}

FString FOnlineFriendUnifiedEOS::GetRealName() const
{
    return this->GetDisplayName(TEXT(""));
}

FString FOnlineFriendUnifiedEOS::GetDisplayName(const FString &Platform) const
{
    auto FriendSystem = this->PlatformHandle->GetSystem<IFriendSystem>();
    auto FriendAlias = FriendSystem->GetFriendAlias(this->LocalUserId, this->Friend->GetUserId());
    if (FriendAlias.IsSet())
    {
        return FriendAlias.GetValue();
    }

    if (this->Friend->GetLinkedEpicGamesAccount().IsSet())
    {
        return this->Friend->GetLinkedEpicGamesAccount()->DisplayNameSanitized;
    }

    for (const auto &KV : this->Friend->GetLinkedExternalAccounts())
    {
        return KV.Value.DisplayName;
    }

    return TEXT("");
}

bool FOnlineFriendUnifiedEOS::GetUserAttribute(const FString &AttrName, FString &OutAttrValue) const
{
    auto RealAttrName = AttrName;

    static bool bDeprecatedPrimaryFriendWarned = false;
    static bool bDeprecatedPreferredFriendWarned = false;
    static bool bDeprecatedSubsystemNamesWarned = false;
    static bool bDeprecatedSubsystemAttributeWarned = false;

    // Backwards compatibility for the deprecated 'eosSynthetic.primaryFriend.subsystemName' attribute.
    if (AttrName == TEXT("eosSynthetic.primaryFriend.subsystemName"))
    {
        if (!bDeprecatedPrimaryFriendWarned)
        {
            UE_LOG(
                LogOnlineSubsystemRedpointEOS,
                Warning,
                TEXT("The 'eosSynthetic.primaryFriend.subsystemName' attribute is now deprecated and will always "
                     "return the first entry listed in the 'runtimePlatform.platformNames' attribute value."));
            bDeprecatedPrimaryFriendWarned = true;
        }
        FString RuntimePlatformNamesValue;
        if (this->GetUserAttribute(TEXT("runtimePlatform.platformNames"), RuntimePlatformNamesValue))
        {
            TArray<FString> RuntimePlatformNames;
            RuntimePlatformNamesValue.ParseIntoArray(RuntimePlatformNames, TEXT(","));
            if (RuntimePlatformNames.Num() > 0)
            {
                OutAttrValue = RuntimePlatformNames[0];
                return true;
            }
        }
        return false;
    }

    // Backwards compatibility for the deprecated 'eosSynthetic.preferredFriend.subsystemName' attribute.
    if (AttrName == TEXT("eosSynthetic.preferredFriend.subsystemName"))
    {
        if (!bDeprecatedPreferredFriendWarned)
        {
            UE_LOG(
                LogOnlineSubsystemRedpointEOS,
                Warning,
                TEXT("The 'eosSynthetic.preferredFriend.subsystemName' attribute is now deprecated and will always "
                     "return the first entry listed in the 'runtimePlatform.platformNames' attribute value."));
            bDeprecatedPreferredFriendWarned = true;
        }
        FString RuntimePlatformNamesValue;
        if (this->GetUserAttribute(TEXT("runtimePlatform.platformNames"), RuntimePlatformNamesValue))
        {
            TArray<FString> RuntimePlatformNames;
            RuntimePlatformNamesValue.ParseIntoArray(RuntimePlatformNames, TEXT(","));
            if (RuntimePlatformNames.Num() > 0)
            {
                OutAttrValue = RuntimePlatformNames[0];
                return true;
            }
        }
        return false;
    }

    // Backwards compatibility for the deprecated 'eosSynthetic.subsystemNames' attribute.
    if (AttrName == TEXT("eosSynthetic.subsystemNames"))
    {
        if (!bDeprecatedSubsystemNamesWarned)
        {
            UE_LOG(
                LogOnlineSubsystemRedpointEOS,
                Warning,
                TEXT("The 'eosSynthetic.subsystemNames' attribute is now deprecated and is no longer guaranteed to be "
                     "a list of OSSv1 subsystem names; the 'runtimePlatform.platformNames' should be used instead."));
            bDeprecatedSubsystemNamesWarned = true;
        }
        return this->GetUserAttribute(TEXT("runtimePlatform.platformNames"), OutAttrValue);
    }

    // Backwards compatibility for the deprecated 'eosSynthetic.friend...' attributes.
    if (AttrName.StartsWith(TEXT("eosSynthetic.friend.")))
    {
        if (!bDeprecatedSubsystemAttributeWarned)
        {
            UE_LOG(
                LogOnlineSubsystemRedpointEOS,
                Warning,
                TEXT("The 'eosSynthetic.friend...' attributes are now deprecated; the 'runtimePlatform.friend...' "
                     "attributes should be used instead."));
            bDeprecatedSubsystemAttributeWarned = true;
        }
        return this->GetUserAttribute(FString::Printf(TEXT("runtimePlatform.%s"), *AttrName.Mid(12)), OutAttrValue);
    }

    // Implement 'deletable' attribute, which isn't accessible in OSSv1 by any other way.
    if (AttrName == TEXT("deletable"))
    {
        auto FriendSystem = this->PlatformHandle->GetSystem<IFriendSystem>();
        OutAttrValue = FriendSystem->IsFriendDeletable(this->LocalUserId, this->Friend->GetUserId()) ? TEXT("true")
                                                                                                     : TEXT("false");
        return true;
    }

    // Implement 'source' attribute, which helps diagnose where friends list entries have come from.
    if (AttrName == TEXT("source"))
    {
        OutAttrValue = this->Source;
        return true;
    }

    auto *AttrValue = this->Friend->GetAttributes().Find(AttrName);
    if (AttrValue != nullptr)
    {
        OutAttrValue = *AttrValue;
        return true;
    }
    return false;
}

bool FOnlineFriendUnifiedEOS::SetUserLocalAttribute(const FString &AttrName, const FString &InAttrValue)
{
    return false;
}

EInviteStatus::Type FOnlineFriendUnifiedEOS::GetInviteStatus() const
{
    return this->InviteState;
}

const FOnlineUserPresence &FOnlineFriendUnifiedEOS::GetPresence() const
{
    return this->PresenceState;
}

FDateTime FOnlineFriendUnifiedEOS::GetLastSeen() const
{
    return this->LastSeen.Get(FDateTime::MinValue());
}

const FUserInfoRef &FOnlineFriendUnifiedEOS::GetFriend() const
{
    return this->Friend;
}

const FString &FOnlineFriendUnifiedEOS::GetSource() const
{
    return this->Source;
}

void FOnlineFriendUnifiedEOS::UpdateFriendAndSource(const FUserInfoRef &InNewUserInfo, const FString &InNewSource)
{
    this->Friend = InNewUserInfo;
    this->Source = InNewSource;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()