// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/RoomRequestedLeave.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3936855958, Redpoint::EOS::Rooms)
{
using namespace ::Redpoint::EOS::Rooms;

class REDPOINTEOSROOMS_API FRoomRequestedLeaveImpl : public IRoomRequestedLeave
{
private:
    FRoomUserId ActorUserId;
    FRoomIdRef RoomId;
    FRoomRef Room;

public:
    FRoomRequestedLeaveImpl(const FRoomUserId &InActorUserId, const FRoomIdRef &InRoomId, const FRoomRef &InRoom);
    UE_NONCOPYABLE(FRoomRequestedLeaveImpl);
    virtual ~FRoomRequestedLeaveImpl() override = default;

    virtual FRoomUserId GetActorUserId() const override;
    virtual FRoomIdRef GetRoomId() const override;
    virtual FRoomRef GetRoom() const override;
};

}

namespace Redpoint::EOS::Rooms
{
REDPOINT_EOS_FILE_NS_EXPORT(3936855958, Redpoint::EOS::Rooms, FRoomRequestedLeaveImpl)
}

REDPOINT_EOS_CODE_GUARD_END()