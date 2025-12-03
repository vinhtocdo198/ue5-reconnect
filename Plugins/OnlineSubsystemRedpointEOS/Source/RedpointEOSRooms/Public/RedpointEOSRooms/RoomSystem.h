// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Requests/CreateRoomRequest.h"
#include "RedpointEOSRooms/Requests/GetCurrentRoomsRequest.h"
#include "RedpointEOSRooms/Requests/JoinRoomRequest.h"
#include "RedpointEOSRooms/Requests/KickRoomMemberRequest.h"
#include "RedpointEOSRooms/Requests/LeaveRoomRequest.h"
#include "RedpointEOSRooms/Requests/PromoteRoomMemberRequest.h"
#include "RedpointEOSRooms/Requests/RestoreRoomInvitesRequest.h"
#include "RedpointEOSRooms/Requests/SearchRoomsRequest.h"
#include "RedpointEOSRooms/Requests/SendRoomInviteRequest.h"
#include "RedpointEOSRooms/Requests/UpdateRoomRequest.h"
#include "RedpointEOSRooms/Room.h"
#include "RedpointEOSRooms/RoomEventContext.h"
#include "RedpointEOSRooms/RoomInvite.h"
#include "RedpointEOSRooms/RoomJoinReason.h"
#include "RedpointEOSRooms/RoomLeaveReason.h"
#include "RedpointEOSRooms/RoomRequestedJoin.h"
#include "RedpointEOSRooms/RoomRequestedLeave.h"
#include "RedpointEOSRooms/RoomSearchResult.h"
#include "RedpointEOSRooms/RoomUpdateInfo.h"
#include "RedpointEOSRooms/RoomUserId.h"
#include "RedpointEOSUserCache/UserInfo.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(195853503, Redpoint::EOS::Rooms)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Requests;
using namespace ::Redpoint::EOS::UserCache;

class REDPOINTEOSROOMS_API IRoomSystem : public IPlatformInstanceSystem
{
public:
    static FName GetSystemName();
    static TSharedRef<IRoomSystem> ConstructSystem(
        FPlatformHandle InPlatformHandle,
        FPlatformInstanceEnvironmentRef InPlatformInstanceEnvironment);

public:
    IRoomSystem() = default;
    UE_NONCOPYABLE(IRoomSystem);
    virtual ~IRoomSystem() override = default;

    typedef TDelegate<void(const FError &ErrorCode, const FRoomPtr &Room)> FOnCreateRoomComplete;
    typedef TDelegate<void(const FError &ErrorCode)> FOnUpdateRoomComplete;
    typedef TDelegate<void(const FError &ErrorCode, const TArray<FRoomSearchResultRef> &RoomSearchResults)>
        FOnSearchRoomsComplete;
    typedef TDelegate<void(const FError &ErrorCode, const FRoomPtr &Room)> FOnJoinRoomComplete;
    typedef TDelegate<void(const FError &ErrorCode)> FOnLeaveRoomComplete;
    typedef TDelegate<void(const FError &ErrorCode)> FOnPromoteRoomMemberComplete;
    typedef TDelegate<void(const FError &ErrorCode)> FOnKickRoomMemberComplete;
    typedef TDelegate<void(const FError &ErrorCode)> FOnRestoreRoomInvitesComplete;
    typedef TDelegate<void(const FError &ErrorCode)> FOnSendRoomInviteComplete;

    typedef TMulticastDelegate<
        void(const FRoomEventContext &EventContext, const FRoomRef &Room, ERoomJoinReason Reason)>
        FOnRoomJoined;
    typedef TMulticastDelegate<
        void(const FRoomEventContext &EventContext, const FRoomRef &Room, ERoomLeaveReason Reason)>
        FOnRoomLeft;
    typedef TMulticastDelegate<
        void(const FRoomEventContext &EventContext, const FRoomRef &Room, const FRoomUpdateInfo &RoomUpdateInfo)>
        FOnRoomUpdated;

    typedef TDelegate<void(const FError &ErrorCode, const FRoomPtr &Room)> FOnAcceptRoomInviteComplete;
    typedef TDelegate<void(const FError &ErrorCode)> FOnRejectRoomInviteComplete;
    typedef TDelegate<void(const FError &ErrorCode)> FOnRejectRoomRequestedJoinComplete;

    enum class ERoomInviteRemovedReason : uint8
    {
        Accepted,
        Rejected,
    };

    typedef TMulticastDelegate<void(const FRoomEventContext &EventContext, const FRoomInviteRef &RoomInvite)>
        FOnRoomInviteReceived;
    typedef TMulticastDelegate<void(
        const FRoomEventContext &EventContext,
        const FRoomInviteRef &RoomInvite,
        ERoomInviteRemovedReason RoomInviteRemovedReason)>
        FOnRoomInviteRemoved;

    typedef TMulticastDelegate<
        void(const FRoomEventContext &EventContext, const FRoomRequestedJoinRef &RoomRequestedJoin)>
        FOnRoomJoinRequested;
    typedef TMulticastDelegate<
        void(const FRoomEventContext &EventContext, const FRoomRequestedLeaveRef &RoomRequestedLeave)>
        FOnRoomLeaveRequested;

    /**
     * Returns the rooms that the local user is currently a member of.
     */
    virtual TArray<FRoomRef> GetCurrentRooms(const FGetCurrentRoomsRequest &Request) = 0;

    /**
     * Creates a room.
     */
    virtual void CreateRoom(const FCreateRoomRequest &Request, const FOnCreateRoomComplete &OnComplete) = 0;

    /**
     * Updates a room that the local user is the owner of.
     */
    virtual void UpdateRoom(
        const FRoomRef &Room,
        const FUpdateRoomRequest &Request,
        const FOnUpdateRoomComplete &OnComplete) = 0;

    /**
     * Searches for available rooms on the backend.
     */
    virtual void SearchRooms(const FSearchRoomsRequest &Request, const FOnSearchRoomsComplete &OnComplete) = 0;

    /**
     * Joins a room using a search result.
     */
    virtual void JoinRoom(const FJoinRoomRequest &Request, const FOnJoinRoomComplete &OnComplete) = 0;

    /**
     * Leaves a room. Depending on the room configuration, this may also destroy the room.
     */
    virtual void LeaveRoom(
        const FRoomRef &Room,
        const FLeaveRoomRequest &Request,
        const FOnLeaveRoomComplete &OnComplete) = 0;

    /**
     * Promotes a member of the room to be the owner. Not all room providers support this operation.
     */
    virtual void PromoteRoomMember(
        const FRoomRef &Room,
        const FPromoteRoomMemberRequest &Request,
        const FOnPromoteRoomMemberComplete &OnComplete) = 0;

    /**
     * Kick a member from the room.
     */
    virtual void KickRoomMember(
        const FRoomRef &Room,
        const FKickRoomMemberRequest &Request,
        const FOnKickRoomMemberComplete &OnComplete) = 0;

    /**
     * The event which fires when a local user joins a room. The room namespace may be NAME_All to receive events across
     * all namespaces.
     */
    virtual FOnRoomJoined &OnRoomJoined(const FName &InRoomNamespace) = 0;

    /**
     * The event which fires when a local user leaves a room. The room namespace may be NAME_All to receive events
     * across all namespaces.
     */
    virtual FOnRoomLeft &OnRoomLeft(const FName &InRoomNamespace) = 0;

    /**
     * The event which fires when a room that a local user is a member of is updated. The room namespace may be NAME_All
     * to receive events across all namespaces.
     */
    virtual FOnRoomUpdated &OnRoomUpdated(const FName &InRoomNamespace) = 0;

    /**
     * Returns the room invites that the local user currently has.
     */
    virtual TArray<FRoomInviteRef> GetCurrentRoomInvites(
        const FRoomUserId &InLocalUser,
        const FName &InRoomNamespace) = 0;

    /**
     * Restore existing room invites and raise the OnRoomInviteReceived event for each room invite restored. This should
     * be used after a user has initially signed in, as it will restore invites sent to the user prior to when the game
     * was running.
     */
    virtual void RestoreRoomInvites(
        const FRestoreRoomInvitesRequest &Request,
        const FOnRestoreRoomInvitesComplete &OnComplete) = 0;

    /**
     * Send an invite to the room to another user.
     */
    virtual void SendRoomInvite(
        const FRoomRef &Room,
        const FSendRoomInviteRequest &Request,
        const FOnSendRoomInviteComplete &OnComplete) = 0;

    /**
     * Reject the specified room invite. Room invites are bound to the local user they are received for, so this
     * function does not accept a local user parameter.
     */
    virtual void RejectRoomInvite(const FRoomInviteRef &RoomInvite, const FOnRejectRoomInviteComplete &OnComplete) = 0;

    /**
     * The event which fires when a new room invite has been received. The room namespace may be NAME_All to receive
     * events across all namespaces.
     */
    virtual FOnRoomInviteReceived &OnRoomInviteReceived(const FName &InRoomNamespace) = 0;

    /**
     * The event which fires when an existing room invite has been removed (because it was consumed via JoinRoom or
     * rejected). The room namespace may be NAME_All to receive events across all namespaces.
     */
    virtual FOnRoomInviteRemoved &OnRoomInviteRemoved(const FName &InRoomNamespace) = 0;

    /**
     * The event which fires when the game needs to join a room, either because an invite was accepted or because the
     * user clicked "Join Lobby" / "Join Session" in the overlay. The room namespace may be NAME_All to receive events
     * across all namespaces.
     */
    virtual FOnRoomJoinRequested &OnRoomJoinRequested(const FName &InRoomNamespace) = 0;

    /**
     * The event which fires when the game needs to leave a room, because the user clicked "Leave Lobby" / "Leave
     * Session" in the overlay. The room namespace may be NAME_All to receive events across all namespaces.
     */
    virtual FOnRoomLeaveRequested &OnRoomLeaveRequested(const FName &InRoomNamespace) = 0;

    /**
     * Returns whether the room system has a room provider with the given name.
     */
    virtual bool HasRoomProvider(const FName &InProviderName) = 0;

    /**
     * Reject the specified requested room join. If the game processes an incoming FRoomRequestedJoinRef from
     * OnRoomJoinRequested() and decides that the search result is not in a state suitable for the local client to join,
     * this call notifies the Epic Games overlay that the join operation was declined and that the operation is no
     * longer in progress. Games that decline requested room joins via this process should still display a visual
     * message indicating why the room join failed, as joins occuring from runtime platforms such as Steam will not have
     * any visual feedback in their overlay.
     */
    virtual void RejectRoomRequestedJoin(
        const FRoomRequestedJoinRef &RoomRequestedJoin,
        const FOnRejectRoomRequestedJoinComplete &OnComplete) = 0;

#if defined(REDPOINT_EOS_ACCESS_INTERNALS)
public:
#else
protected:
#endif
    virtual void OnRoomInviteReceivedFromRuntimePlatform(
        const FRoomUserId &LocalUserId,
        const FUserInfoPtr &SenderUser,
        const FRoomIdRef &RoomId,
        const FName &RoomNamespace) = 0;
    virtual void OnRoomInviteAcceptedFromRuntimePlatform(
        const FRoomUserId &LocalUserId,
        const FUserInfoPtr &SenderUser,
        const FRoomIdRef &RoomId,
        const FName &RoomNamespace) = 0;
    virtual void OnRoomInviteLocallyAcceptedJoinRequiredFromSDKEvent(
        FRoomInviteId RoomInviteId,
        FRoomUserId RoomInviteLocalRecipientUserId) = 0;
};

}

namespace Redpoint::EOS::Rooms
{
REDPOINT_EOS_FILE_NS_EXPORT(195853503, Redpoint::EOS::Rooms, IRoomSystem)
}

REDPOINT_EOS_CODE_GUARD_END()