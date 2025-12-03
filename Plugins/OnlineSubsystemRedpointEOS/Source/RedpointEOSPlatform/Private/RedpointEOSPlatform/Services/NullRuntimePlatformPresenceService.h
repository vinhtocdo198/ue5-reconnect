// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSPlatform/Services/RuntimePlatformPresenceService.h"
#include "RedpointEOSPlatform/Types/PlatformStorage.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(2047259675, Redpoint::EOS::Platform::Services)
{
using namespace ::Redpoint::EOS::UserCache;
using namespace ::Redpoint::EOS::Platform::Services;
using namespace ::Redpoint::EOS::Platform::Types;
using namespace ::Redpoint::EOS::Presence;
using namespace ::Redpoint::EOS::Rooms;

class FNullRuntimePlatformPresenceService : public IRuntimePlatformPresenceService
{
private:
    struct FPlatformStorage
    {
        FOnRoomAdvertisementInviteReceived OnRoomAdvertisementInviteReceivedDelegate;
        FOnRoomAdvertisementInviteAccepted OnRoomAdvertisementInviteAcceptedDelegate;
        FOnNonUnifiedFriendPresenceChanged OnNonUnifiedFriendPresenceChangedDelegate;
    };
    TPlatformStorage<FPlatformStorage> PlatformStorage;

public:
    FNullRuntimePlatformPresenceService() = default;
    UE_NONCOPYABLE(FNullRuntimePlatformPresenceService);
    virtual ~FNullRuntimePlatformPresenceService() override = default;

    virtual TOptional<bool> IsLocalUserOnline(const FRuntimePlatformServiceCallContextRef &CallContext) const override;

    virtual void UpdateLocalUserPresence(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const FPresenceUserState &InDesiredPresenceState,
        const FOnLocalUserPresenceUpdated &InCompletionDelegate) override;

    virtual void UpsertRoomAdvertisement(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const FPresenceUserAdvertisedRoom &InAdvertisedRoom,
        bool bIsParty,
        const FOnUpsertRoomAdvertisementComplete &InCompletionDelegate) override;

    virtual void DeleteRoomAdvertisement(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const FRoomIdRef &InRoomId,
        bool bIsParty,
        const FOnDeleteRoomAdvertisementComplete &InCompletionDelegate) override;

    virtual FOnRoomAdvertisementInviteReceived &OnRoomAdvertisementInviteReceived(
        const FRuntimePlatformServiceCallContextRef &CallContext) override;

    virtual FOnRoomAdvertisementInviteAccepted &OnRoomAdvertisementInviteAccepted(
        const FRuntimePlatformServiceCallContextRef &CallContext) override;

    virtual TOptional<EPresenceUserActivityState> GetNonUnifiedFriendPresence(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const EOS_EExternalAccountType &FriendAccountType,
        const FString &FriendAccountId) override;

    virtual FOnNonUnifiedFriendPresenceChanged &OnNonUnifiedFriendPresenceChanged(
        const FRuntimePlatformServiceCallContextRef &CallContext) override;

    virtual void SendRoomInviteToNonUnifiedFriend(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const EOS_EExternalAccountType &FriendAccountType,
        const FString &FriendAccountId,
        const FRoomIdRef &RoomId,
        const FOnSendRoomInviteToNonUnifiedFriendComplete &InCompletionDelegate) override;

    virtual bool TryOpenRoomInviteRecipientSelection(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const FRoomIdRef &RoomId) override;
};

extern TSharedRef<IRuntimePlatformPresenceService> NullPresenceService;

}

namespace Redpoint::EOS::Platform::Services
{
REDPOINT_EOS_FILE_NS_EXPORT(2047259675, Redpoint::EOS::Platform::Services, FNullRuntimePlatformPresenceService)
REDPOINT_EOS_FILE_NS_EXPORT(2047259675, Redpoint::EOS::Platform::Services, NullPresenceService)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()