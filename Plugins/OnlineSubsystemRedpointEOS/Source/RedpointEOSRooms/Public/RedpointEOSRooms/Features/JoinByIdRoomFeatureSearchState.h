// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Features/JoinByIdRoomFeature.h"
#include "RedpointEOSRooms/Features/RoomFeatureSearchState.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3575841671, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

class REDPOINTEOSROOMS_API IJoinByIdRoomFeatureSearchState : public IRoomFeatureSearchState<IJoinByIdRoomFeature>
{
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(3575841671, Redpoint::EOS::Rooms::Features, IJoinByIdRoomFeatureSearchState)
}

REDPOINT_EOS_CODE_GUARD_END()