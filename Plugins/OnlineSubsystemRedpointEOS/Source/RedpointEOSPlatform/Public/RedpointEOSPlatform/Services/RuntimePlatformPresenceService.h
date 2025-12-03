// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Online/CoreOnline.h"
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSPlatform/Services/ForwardDecls.h"
#include "RedpointEOSPlatform/Services/RuntimePlatformServiceCallContext.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1312836081, Redpoint::EOS::Platform::Services)
{
using namespace ::Redpoint::EOS::UserCache;
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Platform::Services;
using namespace ::Redpoint::EOS::Presence;
using namespace ::Redpoint::EOS::Rooms;

class REDPOINTEOSPLATFORM_API IRuntimePlatformPresenceService
{
public:
    IRuntimePlatformPresenceService() = default;
    UE_NONCOPYABLE(IRuntimePlatformPresenceService);
    virtual ~IRuntimePlatformPresenceService() = default;

    /**
     * Called by the cross-platform presence system to determine if the local platform wants the local user to appear
     * online or offline. If this platform integration does not control cross-platform presence, it should return an
     * unset value for the TOptional<bool>. Only the first integration that returns a set value will be used to
     * determine if cross-platform presence should be active.
     */
    virtual TOptional<bool> IsLocalUserOnline(const FRuntimePlatformServiceCallContextRef &CallContext) const = 0;

    /**
     * The callback type used for signalling completion of the UpdateLocalUserPresence call.
     */
    typedef TDelegate<void(FError ErrorCode)> FOnLocalUserPresenceUpdated;

    /**
     * Called by the cross-platform presence system to update the advertised presence information on the local platform.
     * The desired presence state contains all of the information needed to advertise local platform presence, though
     * not all local platforms will use all values.
     */
    virtual void UpdateLocalUserPresence(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const FPresenceUserState &InDesiredPresenceState,
        const FOnLocalUserPresenceUpdated &InCompletionDelegate) = 0;

    /**
     * The callback type used for signalling completion of the UpsertRoomAdvertisement call.
     */
    typedef TDelegate<void(FError ErrorCode)> FOnUpsertRoomAdvertisementComplete;

    /**
     * Create or update the room advertisement for the room advertised by presence. This should create or update the
     * underlying session or party on the runtime platform such that the user's presence indicates they are in a session
     * or party.
     *
     * It is expected that implementors of this function will search for an existing room advertisement for the
     * specified room and join it, only creating the room advertisement if no other user is currently advertising for
     * this room.
     */
    virtual void UpsertRoomAdvertisement(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const FPresenceUserAdvertisedRoom &InAdvertisedRoom,
        bool bIsParty,
        const FOnUpsertRoomAdvertisementComplete &InCompletionDelegate) = 0;

    /**
     * The callback type used for signalling completion of the DeleteRoomAdvertisement call.
     */
    typedef TDelegate<void(FError ErrorCode)> FOnDeleteRoomAdvertisementComplete;

    /**
     * Delete the existing room advertisement based on the old room ID. This should delete the underlying session or
     * party on the runtime platform.
     */
    virtual void DeleteRoomAdvertisement(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const FRoomIdRef &InRoomId,
        bool bIsParty,
        const FOnDeleteRoomAdvertisementComplete &InCompletionDelegate) = 0;

    /**
     * The delegate type used for the OnRoomAdvertisementInviteReceived. Note that the sender user is not always
     * available.
     */
    typedef TMulticastDelegate<void(
        const UE::Online::FAccountId &LocalUserId,
        const FUserInfoPtr &SenderUser,
        const FRoomIdRef &RoomId,
        const FName &RoomNamespace)>
        FOnRoomAdvertisementInviteReceived;

    /**
     * This event is fired when another user on the runtime platform invites a local user to join that room
     * advertisement. For example, if Steam user A is playing the game and has their EOS session or party room
     * advertised on Steam via the implementor of IRuntimePlatformPresenceService, then when user A uses Steam's own
     * "Send Invite" UI to send an invite to Steam user B, this event will fire for user B with the original EOS room
     * ID.
     *
     * Because this situation can happen when the game is not running, and the game may be launched in response to the
     * user undertaking these actions, there must be a call to RestoreInvitesAfterLogin() which will cause both
     * OnRoomAdvertisementInviteReceived() and OnRoomAdvertisementInviteAccepted() to fire for any buffered invite
     * operations that occurred while the game wasn't running.
     */
    virtual FOnRoomAdvertisementInviteReceived &OnRoomAdvertisementInviteReceived(
        const FRuntimePlatformServiceCallContextRef &CallContext) = 0;

    /**
     * The delegate type used for the OnRoomAdvertisementInviteAccepted. Note that the sender user is not always
     * available.
     */
    typedef TMulticastDelegate<void(
        const UE::Online::FAccountId &LocalUserId,
        const FUserInfoPtr &SenderUser,
        const FRoomIdRef &RoomId,
        const FName &RoomNamespace)>
        FOnRoomAdvertisementInviteAccepted;

    /**
     * This event is fired when a local user on the runtime platform accepts an invite to another user's room
     * advertisement, or directly joins another user's room advertisement. For example, if Steam user A is playing the
     * game and has their EOS session or party room advertised on Steam via the implementor of
     * IRuntimePlatformPresenceService, then when Steam user B clicks "Join Game" or "Join Party" on Steam user A's
     * profile in the Steam friends list, this this event will fire for user B with the original EOS room ID.
     *
     * Because this situation can happen when the game is not running, and the game may be launched in response to the
     * user undertaking these actions, there must be a call to RestoreInvitesAfterLogin() which will cause both
     * OnRoomAdvertisementInviteReceived() and OnRoomAdvertisementInviteAccepted() to fire for any buffered invite
     * operations that occurred while the game wasn't running.
     */
    virtual FOnRoomAdvertisementInviteAccepted &OnRoomAdvertisementInviteAccepted(
        const FRuntimePlatformServiceCallContextRef &CallContext) = 0;

    /**
     * Get the presence activity state for a non-unified friend of the local user.
     */
    virtual TOptional<EPresenceUserActivityState> GetNonUnifiedFriendPresence(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const EOS_EExternalAccountType &FriendAccountType,
        const FString &FriendAccountId) = 0;

    /**
     * The delegate type used for the OnNonUnifiedFriendPresenceChanged.
     */
    typedef TMulticastDelegate<void(
        int LocalUserNum,
        const EOS_EExternalAccountType &FriendAccountType,
        const FString &FriendAccountId,
        EPresenceUserActivityState NewPresenceActivityState)>
        FOnNonUnifiedFriendPresenceChanged;

    /**
     * This event is fired when the presence changes for a non-unified friend of the local user.
     */
    virtual FOnNonUnifiedFriendPresenceChanged &OnNonUnifiedFriendPresenceChanged(
        const FRuntimePlatformServiceCallContextRef &CallContext) = 0;

    /**
     * The callback type used for signalling completion of the SendRoomInviteToNonUnifiedFriend call.
     */
    typedef TDelegate<void(FError ErrorCode)> FOnSendRoomInviteToNonUnifiedFriendComplete;

    /**
     * Send an invite to a friend over the local platform, if possible. Note that the target user doesn't actually have
     * to be non-unified; even if there is a PUID associated with the target, this call should still work.
     */
    virtual void SendRoomInviteToNonUnifiedFriend(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const EOS_EExternalAccountType &FriendAccountType,
        const FString &FriendAccountId,
        const FRoomIdRef &RoomId,
        const FOnSendRoomInviteToNonUnifiedFriendComplete &InCompletionDelegate) = 0;

    /**
     * Try to open the platform overlay for the user to send a room invite to one or more friends. There is no callback
     * for this event because the user may spend an arbitrary amount of time in the platform's user interface. Returns
     * whether or not the overlay could be successfully opened.
     */
    virtual bool TryOpenRoomInviteRecipientSelection(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const FRoomIdRef &RoomId) = 0;
};

}

namespace Redpoint::EOS::Platform::Services
{
REDPOINT_EOS_FILE_NS_EXPORT(1312836081, Redpoint::EOS::Platform::Services, IRuntimePlatformPresenceService)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()