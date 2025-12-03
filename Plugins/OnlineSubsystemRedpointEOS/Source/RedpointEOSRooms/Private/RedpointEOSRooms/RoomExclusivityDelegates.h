// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Providers/RoomProviderFeatureHandlerBase.h"
#include "RedpointEOSRooms/RoomScope.h"
#include "RedpointEOSRooms/RoomUserId.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(404774264, Redpoint::EOS::Rooms)
{
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Providers;

typedef TDelegate<
    FRoomPtr(ERoomScope RoomScope, FRoomUserId UserId, FRoomProviderFeatureHandlerBase::FExclusivityKey ExclusivityKey)>
    FGetExistingRoomForExclusivityKey;

}

namespace Redpoint::EOS::Rooms
{
REDPOINT_EOS_FILE_NS_EXPORT(404774264, Redpoint::EOS::Rooms, FGetExistingRoomForExclusivityKey)
}

REDPOINT_EOS_CODE_GUARD_END()