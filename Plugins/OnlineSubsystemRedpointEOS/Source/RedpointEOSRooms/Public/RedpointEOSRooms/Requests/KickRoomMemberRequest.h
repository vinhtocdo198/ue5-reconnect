// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/RoomUserId.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(4234120463, Redpoint::EOS::Rooms::Requests)
{
using namespace ::Redpoint::EOS::Rooms;

class REDPOINTEOSROOMS_API FKickRoomMemberRequest
{
public:
    /**
     * The target user to kick from the room.
     */
    FRoomUserId TargetUser;
};

}

namespace Redpoint::EOS::Rooms::Requests
{
REDPOINT_EOS_FILE_NS_EXPORT(4234120463, Redpoint::EOS::Rooms::Requests, FKickRoomMemberRequest)
}

REDPOINT_EOS_CODE_GUARD_END()