// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Features/RoomFeatureSearchState.h"
#include "RedpointEOSRooms/Features/SanctionEnforcementRoomFeature.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1651688544, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

class REDPOINTEOSROOMS_API ISanctionEnforcementRoomFeatureSearchState
    : public IRoomFeatureSearchState<ISanctionEnforcementRoomFeature>
{
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(1651688544, Redpoint::EOS::Rooms::Features, ISanctionEnforcementRoomFeatureSearchState)
}

REDPOINT_EOS_CODE_GUARD_END()