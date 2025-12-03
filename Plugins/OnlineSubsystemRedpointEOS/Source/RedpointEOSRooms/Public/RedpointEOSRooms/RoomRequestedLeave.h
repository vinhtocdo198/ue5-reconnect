// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Room.h"
#include "RedpointEOSRooms/RoomSearchResult.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3206536946, Redpoint::EOS::Rooms)
{
using namespace ::Redpoint::EOS::Rooms;

class REDPOINTEOSROOMS_API IRoomRequestedLeave
{
public:
    IRoomRequestedLeave() = default;
    UE_NONCOPYABLE(IRoomRequestedLeave);
    virtual ~IRoomRequestedLeave() = default;

    /**
     * Returns the user ID that should perform the LeaveRoom operation.
     */
    virtual FRoomUserId GetActorUserId() const = 0;

    /**
     * Returns the room ID associated with the request.
     */
    virtual FRoomIdRef GetRoomId() const = 0;

    /**
     * Returns the room associated with the request. This may not be available in all cases.
     */
    virtual FRoomRef GetRoom() const = 0;
};

typedef TSharedRef<IRoomRequestedLeave> FRoomRequestedLeaveRef;
typedef TSharedPtr<IRoomRequestedLeave> FRoomRequestedLeavePtr;

}

namespace Redpoint::EOS::Rooms
{
REDPOINT_EOS_FILE_NS_EXPORT(3206536946, Redpoint::EOS::Rooms, IRoomRequestedLeave)
REDPOINT_EOS_FILE_NS_EXPORT(3206536946, Redpoint::EOS::Rooms, FRoomRequestedLeaveRef)
REDPOINT_EOS_FILE_NS_EXPORT(3206536946, Redpoint::EOS::Rooms, FRoomRequestedLeavePtr)
}

REDPOINT_EOS_CODE_GUARD_END()