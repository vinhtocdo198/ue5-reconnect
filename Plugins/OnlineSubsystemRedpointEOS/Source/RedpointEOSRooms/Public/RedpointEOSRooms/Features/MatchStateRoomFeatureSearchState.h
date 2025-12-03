// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Features/MatchStateRoomFeature.h"
#include "RedpointEOSRooms/Features/RoomFeatureSearchState.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(755294687, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

class REDPOINTEOSROOMS_API IMatchStateRoomFeatureSearchState : public IRoomFeatureSearchState<IMatchStateRoomFeature>
{
public:
    virtual bool AllowJoinInProgress() const = 0;
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(755294687, Redpoint::EOS::Rooms::Features, IMatchStateRoomFeatureSearchState)
}

REDPOINT_EOS_CODE_GUARD_END()