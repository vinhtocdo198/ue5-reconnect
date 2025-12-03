// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/RoomScope.h"
#include "RedpointEOSRooms/RoomUserId.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1926869330, Redpoint::EOS::Rooms::Database)
{
using namespace ::Redpoint::EOS::Rooms;

typedef TTuple<ERoomScope, FRoomUserId> FRoomScopeAndRoomUserId;

}

namespace Redpoint::EOS::Rooms::Database
{
REDPOINT_EOS_FILE_NS_EXPORT(1926869330, Redpoint::EOS::Rooms::Database, FRoomScopeAndRoomUserId)
}

REDPOINT_EOS_CODE_GUARD_END()