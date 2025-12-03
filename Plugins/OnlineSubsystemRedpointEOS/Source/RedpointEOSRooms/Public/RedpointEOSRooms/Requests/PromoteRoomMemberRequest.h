// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/RoomUserId.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2370156678, Redpoint::EOS::Rooms::Requests)
{
using namespace ::Redpoint::EOS::Rooms;

class REDPOINTEOSROOMS_API FPromoteRoomMemberRequest
{
public:
    /**
     * The target user to promote to the room owner.
     */
    FRoomUserId TargetUser;
};

}

namespace Redpoint::EOS::Rooms::Requests
{
REDPOINT_EOS_FILE_NS_EXPORT(2370156678, Redpoint::EOS::Rooms::Requests, FPromoteRoomMemberRequest)
}

REDPOINT_EOS_CODE_GUARD_END()