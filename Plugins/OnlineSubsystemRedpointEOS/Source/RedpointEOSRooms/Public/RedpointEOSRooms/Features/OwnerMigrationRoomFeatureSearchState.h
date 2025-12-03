// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Features/OwnerMigrationRoomFeature.h"
#include "RedpointEOSRooms/Features/RoomFeatureSearchState.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2499548475, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

class REDPOINTEOSROOMS_API IOwnerMigrationRoomFeatureSearchState
    : public IRoomFeatureSearchState<IOwnerMigrationRoomFeature>
{
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(2499548475, Redpoint::EOS::Rooms::Features, IOwnerMigrationRoomFeatureSearchState)
}

REDPOINT_EOS_CODE_GUARD_END()