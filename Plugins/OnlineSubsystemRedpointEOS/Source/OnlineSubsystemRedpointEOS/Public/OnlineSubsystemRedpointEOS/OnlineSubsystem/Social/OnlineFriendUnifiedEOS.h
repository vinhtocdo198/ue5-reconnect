// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "Interfaces/OnlinePresenceInterface.h"
#include "OnlineSubsystemTypes.h"
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSIdentity/IdentityUserId.h"
#include "RedpointEOSUserCache/UserInfo.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3115511217, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Social)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::UserCache;

class ONLINESUBSYSTEMREDPOINTEOS_API FOnlineFriendUnifiedEOS : public FOnlineFriend,
                                                               public FOnlineRecentPlayer,
                                                               public FOnlineBlockedPlayer,
                                                               public TSharedFromThis<FOnlineFriendUnifiedEOS>
{
private:
    FPlatformHandle PlatformHandle;
    FIdentityUserId LocalUserId;
    FUserInfoRef Friend;
    FString Source;

public:
    FOnlineUserPresence PresenceState;
    EInviteStatus::Type InviteState;
    TOptional<FDateTime> LastSeen;

public:
    FOnlineFriendUnifiedEOS(
        const FPlatformHandle &InPlatformHandle,
        const FIdentityUserId &InLocalUserId,
        const FUserInfoRef &InFriend,
        const FString &InSource);
    UE_NONCOPYABLE(FOnlineFriendUnifiedEOS);
    virtual ~FOnlineFriendUnifiedEOS() override = default;

    virtual FUniqueNetIdRef GetUserId() const override;
    virtual FString GetRealName() const override;
    virtual FString GetDisplayName(const FString &Platform) const override;
    virtual bool GetUserAttribute(const FString &AttrName, FString &OutAttrValue) const override;
    virtual bool SetUserLocalAttribute(const FString &AttrName, const FString &InAttrValue) override;
    virtual EInviteStatus::Type GetInviteStatus() const override;
    virtual const FOnlineUserPresence &GetPresence() const override;
    virtual FDateTime GetLastSeen() const override;

    const FUserInfoRef &GetFriend() const;

    const FString &GetSource() const;
    void UpdateFriendAndSource(const FUserInfoRef &InNewUserInfo, const FString &InNewSource);
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Social
{
REDPOINT_EOS_FILE_NS_EXPORT(
    3115511217,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Social,
    FOnlineFriendUnifiedEOS);
}

#endif

REDPOINT_EOS_CODE_GUARD_END()