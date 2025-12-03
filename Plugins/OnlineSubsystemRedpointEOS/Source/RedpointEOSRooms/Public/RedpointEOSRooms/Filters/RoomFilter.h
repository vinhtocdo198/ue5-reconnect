// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Filters/RoomFilterAction.h"
#include "RedpointEOSRooms/Requests/LeaveRoomRequest.h"
#include "RedpointEOSRooms/Room.h"
#include "RedpointEOSRooms/RoomSystem.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2482032822, Redpoint::EOS::Rooms::Filters)
{
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Filters;
using namespace ::Redpoint::EOS::Rooms::Requests;
using namespace ::Redpoint::EOS::API;

class REDPOINTEOSROOMS_API IRoomFilter
{
public:
    IRoomFilter() = default;
    UE_NONCOPYABLE(IRoomFilter);
    virtual ~IRoomFilter() = default;

    virtual FString GetFilterDebugName() const = 0;

    /**
     * Called when a the user is about to leave a room due to LeaveRoom being called. This happens when the game
     * explicitly requests a room leave, possibly in response to OnRoomLeaveRequested from the overlay.
     */
    virtual ERoomFilterAction OnLeaveRoom(
        const FPlatformHandle &PlatformHandle,
        const FRoomRef &Room,
        FLeaveRoomRequest &Request,
        const IRoomSystem::FOnLeaveRoomComplete &OnComplete);

    /**
     * Called after the user has left a room due to an external reason (such as a disconnection or being kicked). This
     * does not fire when the game explicitly requests a room leave. When this function is called, the room has already
     * been left, as the plugin is responding to the user already being removed by the EOS SDK.
     *
     * However, the provided room is still present locally for the duration of this call, so you can inspect and
     * initiate another room request in response to it. If the room request you initiate would conflict with the
     * broadcasting of the room left event, it may be deferred until after your room request resolves, which can help
     * you prevent the game observing a missing room at any point.
     */
    virtual void OnRoomLeftExternally(
        const FPlatformHandle &PlatformHandle,
        const FRoomRef &Room,
        ERoomLeaveReason RoomLeaveReason);
};

}

namespace Redpoint::EOS::Rooms::Filters
{
REDPOINT_EOS_FILE_NS_EXPORT(2482032822, Redpoint::EOS::Rooms::Filters, IRoomFilter)
}

REDPOINT_EOS_CODE_GUARD_END()