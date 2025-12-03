// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Filters/RoomFilter.h"

#include "RedpointEOSAPI/Memory.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2482032822, Redpoint::EOS::Rooms::Filters)
{

ERoomFilterAction IRoomFilter::OnLeaveRoom(
    const FPlatformHandle &PlatformHandle,
    const FRoomRef &Room,
    FLeaveRoomRequest &Request,
    const IRoomSystem::FOnLeaveRoomComplete &OnComplete)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Filters);

    return ERoomFilterAction::Continue;
}

void IRoomFilter::OnRoomLeftExternally(
    const FPlatformHandle &PlatformHandle,
    const FRoomRef &Room,
    ERoomLeaveReason RoomLeaveReason)
{
}

}

REDPOINT_EOS_CODE_GUARD_END()