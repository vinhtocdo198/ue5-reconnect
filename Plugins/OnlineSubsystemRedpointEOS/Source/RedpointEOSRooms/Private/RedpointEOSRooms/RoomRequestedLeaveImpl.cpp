// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/RoomRequestedLeaveImpl.h"

#include "RedpointEOSAPI/Memory.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3936855958, Redpoint::EOS::Rooms)
{
using namespace ::Redpoint::EOS::Rooms;

FRoomRequestedLeaveImpl::FRoomRequestedLeaveImpl(
    const FRoomUserId &InActorUserId,
    const FRoomIdRef &InRoomId,
    const FRoomRef &InRoom)
    : ActorUserId(InActorUserId)
    , RoomId(InRoomId)
    , Room(InRoom)
{
}

FRoomUserId FRoomRequestedLeaveImpl::GetActorUserId() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    return this->ActorUserId;
}

FRoomIdRef FRoomRequestedLeaveImpl::GetRoomId() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    return this->RoomId;
}

FRoomRef FRoomRequestedLeaveImpl::GetRoom() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    return this->Room;
}

}

REDPOINT_EOS_CODE_GUARD_END()