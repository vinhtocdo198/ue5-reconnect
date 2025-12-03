// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Database/RoomDatabaseExclusivityState.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3891395109, Redpoint::EOS::Rooms::Database)
{

FRoomDatabaseExclusivityState::FRoomDatabaseExclusivityState(const FString &InExclusiveKey)
    : ReservedForTransactionId(0)
    , ReservedForLeaveRoom(false)
    , AssignAtTransactionEnd(false)
    , Room()
    , ExclusiveKey(InExclusiveKey)
{
}

}

REDPOINT_EOS_CODE_GUARD_END()