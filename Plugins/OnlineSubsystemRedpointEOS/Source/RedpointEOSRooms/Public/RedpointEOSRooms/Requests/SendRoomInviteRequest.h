// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/RoomUserId.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1693717373, Redpoint::EOS::Rooms::Requests)
{
using namespace ::Redpoint::EOS::Rooms;

class REDPOINTEOSROOMS_API FSendRoomInviteRequest
{
public:
    /**
     * The target user to invite to the room.
     */
    FRoomUserId TargetUser;
};

}

namespace Redpoint::EOS::Rooms::Requests
{
REDPOINT_EOS_FILE_NS_EXPORT(1693717373, Redpoint::EOS::Rooms::Requests, FSendRoomInviteRequest)
}

REDPOINT_EOS_CODE_GUARD_END()