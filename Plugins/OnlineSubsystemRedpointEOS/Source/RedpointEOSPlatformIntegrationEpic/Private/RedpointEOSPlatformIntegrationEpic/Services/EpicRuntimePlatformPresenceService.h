// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "RedpointEOSAPI/EventHandle.h"
#include "RedpointEOSPlatform/Services/RuntimePlatformPresenceService.h"
#include "RedpointEOSPresence/PresenceUserAdvertisedRoom.h"
#include "RedpointEOSPresence/PresenceUserState.h"
#include "RedpointEOSRooms/RoomId.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3348887799, Redpoint::EOS::Platform::Integration::Epic::Services)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Platform::Services;
using namespace ::Redpoint::EOS::Presence;
using namespace ::Redpoint::EOS::Rooms;

class FEpicRuntimePlatformPresenceService : public IRuntimePlatformPresenceService,
                                            public TSharedFromThis<FEpicRuntimePlatformPresenceService>
{
private:
    class FDelegateStorage
    {
    public:
        TMap<int32, FOnRoomAdvertisementInviteReceived> OnRoomAdvertisementInviteReceivedDelegate;
        TMap<int32, FOnRoomAdvertisementInviteAccepted> OnRoomAdvertisementInviteAcceptedDelegate;
        TMap<int32, FOnNonUnifiedFriendPresenceChanged> OnNonUnifiedFriendPresenceChangedDelegate;
        FEventHandlePtr OnPresenceChanged;
    };
    TMap<FPlatformHandle, FDelegateStorage> PresenceDelegatesByPlatform;

public:
    FEpicRuntimePlatformPresenceService();
    UE_NONCOPYABLE(FEpicRuntimePlatformPresenceService);
    virtual ~FEpicRuntimePlatformPresenceService() override = default;

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

private:
    void OnPlatformShutdown(FPlatformInstance &Instance, FPlatformHandle PlatformHandle);

public:
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

    void RegisterEvents(const FRuntimePlatformServiceCallContextRef &CallContext, const FSimpleDelegate &OnComplete);

    void UnregisterEvents(const FRuntimePlatformServiceCallContextRef &CallContext);
};

}

namespace Redpoint::EOS::Platform::Integration::Epic::Services
{
REDPOINT_EOS_FILE_NS_EXPORT(
    3348887799,
    Redpoint::EOS::Platform::Integration::Epic::Services,
    FEpicRuntimePlatformPresenceService)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()