// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Requests/GetCurrentRoomsRequest.h"
#include "RedpointEOSRooms/RoomUserId.h"

namespace REDPOINT_EOS_FILE_NS_ID(2073920046, Redpoint::EOS::Tests::RoomSystem)
{
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Requests;

FGetCurrentRoomsRequest GetCurrentRoomsRequest(const FRoomUserId &RoomUserId);

}

namespace Redpoint::EOS::Tests::RoomSystem
{
REDPOINT_EOS_FILE_NS_EXPORT(2073920046, Redpoint::EOS::Tests::RoomSystem, GetCurrentRoomsRequest)
}