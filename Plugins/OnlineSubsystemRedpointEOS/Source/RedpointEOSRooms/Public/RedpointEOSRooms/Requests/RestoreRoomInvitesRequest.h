// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/RoomUserId.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(46561547, Redpoint::EOS::Rooms::Requests)
{
using namespace ::Redpoint::EOS::Rooms;

class REDPOINTEOSROOMS_API FRestoreRoomInvitesRequest
{
public:
    /**
     * The target user to restore room invites for.
     */
    FRoomUserId LocalUser;
};

}

namespace Redpoint::EOS::Rooms::Requests
{
REDPOINT_EOS_FILE_NS_EXPORT(46561547, Redpoint::EOS::Rooms::Requests, FRestoreRoomInvitesRequest)
}

REDPOINT_EOS_CODE_GUARD_END()