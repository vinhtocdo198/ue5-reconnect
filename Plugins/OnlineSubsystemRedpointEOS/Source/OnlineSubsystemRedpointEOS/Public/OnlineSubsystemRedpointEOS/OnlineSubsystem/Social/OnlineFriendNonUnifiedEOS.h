// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "Interfaces/OnlinePresenceInterface.h"
#include "OnlineSubsystemTypes.h"
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSIdentity/IdentityUserId.h"
#include "RedpointEOSUserCache/ExternalUserInfo.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1131179970, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Social)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::UserCache;

class ONLINESUBSYSTEMREDPOINTEOS_API FOnlineFriendNonUnifiedEOS : public FOnlineFriend,
                                                                  public TSharedFromThis<FOnlineFriendNonUnifiedEOS>
{
private:
    FUniqueNetIdRef UniqueNetId;
    FPlatformHandle PlatformHandle;
    FIdentityUserId LocalUserId;
    FExternalUserInfoRef Friend;

public:
    FOnlineUserPresence PresenceState;
    EInviteStatus::Type InviteState;

public:
    FOnlineFriendNonUnifiedEOS(
        const FUniqueNetIdRef &InUniqueNetId,
        const FPlatformHandle &InPlatformHandle,
        const FIdentityUserId &InLocalUserId,
        const FExternalUserInfoRef &InFriend);
    UE_NONCOPYABLE(FOnlineFriendNonUnifiedEOS);
    virtual ~FOnlineFriendNonUnifiedEOS() override = default;

    virtual FUniqueNetIdRef GetUserId() const override;
    virtual FString GetRealName() const override;
    virtual FString GetDisplayName(const FString &Platform) const override;
    virtual bool GetUserAttribute(const FString &AttrName, FString &OutAttrValue) const override;
    virtual bool SetUserLocalAttribute(const FString &AttrName, const FString &InAttrValue) override;
    virtual EInviteStatus::Type GetInviteStatus() const override;
    virtual const FOnlineUserPresence &GetPresence() const override;

    const FExternalUserInfoRef &GetFriend() const;
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Social
{
REDPOINT_EOS_FILE_NS_EXPORT(
    1131179970,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Social,
    FOnlineFriendNonUnifiedEOS);
}

#endif

REDPOINT_EOS_CODE_GUARD_END()