// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_META_ENABLED
#include "RedpointEOSCore/Utils/HasEventRegistration.h"
#include "RedpointEOSCore/Utils/PeriodicTimer.h"
#include "RedpointEOSPlatform/Services/RuntimePlatformPresenceService.h"
#include "RedpointEOSPlatformIntegrationMeta/MetaSDK.h"
#include "RedpointEOSPresence/PresenceUserAdvertisedRoom.h"
#include "RedpointEOSRooms/Room.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_META_ENABLED

namespace REDPOINT_EOS_FILE_NS_ID(3098476363, Redpoint::EOS::Platform::Integration::Meta::Services)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Core::Utils;
using namespace ::Redpoint::EOS::Platform::Integration::Meta;
using namespace ::Redpoint::EOS::Platform::Services;
using namespace ::Redpoint::EOS::Presence;
using namespace ::Redpoint::EOS::Rooms;

class FMetaRuntimePlatformPresenceService : public IRuntimePlatformPresenceService,
                                            public IHasEventRegistration,
                                            public TSharedFromThis<FMetaRuntimePlatformPresenceService>
{
private:
    FOnRoomAdvertisementInviteReceived OnRoomAdvertisementInviteReceivedDelegate;
    FOnRoomAdvertisementInviteAccepted OnRoomAdvertisementInviteAcceptedDelegate;
    FOnNonUnifiedFriendPresenceChanged OnNonUnifiedFriendPresenceChangedDelegate;
    TSharedRef<FPeriodicTimer> RefreshFriendPresenceTimer;
    TMap<ovrID, EPresenceUserActivityState> FriendPresenceState;
    TSharedPtr<FPlatformInstance> PlatformHandle;
    TArray<FString> DeeplinkBuffer;
    TOptional<FPresenceUserAdvertisedRoom> CurrentlyAdvertisedParty;
    TOptional<FPresenceUserAdvertisedRoom> CurrentlyAdvertisedSession;

public:
    FMetaRuntimePlatformPresenceService();
    UE_NONCOPYABLE(FMetaRuntimePlatformPresenceService);
    virtual ~FMetaRuntimePlatformPresenceService() override = default;

    virtual void RegisterEvents() override;

    void SetPlatformHandle(const FPlatformHandle &PlatformHandle);
    void FlushDeeplinkEventsAfterLogin();

private:
    void RefreshFriendPresence();
    void OnGetLoggedInUserFriendsComplete(const FError &ErrorCode, const TOptional<TArray<ovrUserHandle>> &Friends);
    void OnDeeplinkReceived(const FString &Deeplink);

public:
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

#if REDPOINT_EOS_META_HAS_GROUP_PRESENCE_SET
private:
    void SyncRoomAdvertisement(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const FOnUpsertRoomAdvertisementComplete &InCompletionDelegate);
#endif

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
};

}

namespace Redpoint::EOS::Platform::Integration::Meta::Services
{
REDPOINT_EOS_FILE_NS_EXPORT(
    3098476363,
    Redpoint::EOS::Platform::Integration::Meta::Services,
    FMetaRuntimePlatformPresenceService)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()