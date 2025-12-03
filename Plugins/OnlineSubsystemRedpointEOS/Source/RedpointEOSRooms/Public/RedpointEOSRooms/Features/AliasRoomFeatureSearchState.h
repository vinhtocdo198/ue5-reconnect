// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Features/AliasRoomFeature.h"
#include "RedpointEOSRooms/Features/RoomFeatureSearchState.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(666484014, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

class REDPOINTEOSROOMS_API IAliasRoomFeatureSearchState : public IRoomFeatureSearchState<IAliasRoomFeature>
{
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(666484014, Redpoint::EOS::Rooms::Features, IAliasRoomFeatureSearchState)
}

REDPOINT_EOS_CODE_GUARD_END()