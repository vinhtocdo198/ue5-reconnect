// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Features/PresenceRoomFeature.h"
#include "RedpointEOSRooms/Features/RoomFeatureSearchState.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(4059745728, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

class REDPOINTEOSROOMS_API IPresenceRoomFeatureSearchState : public IRoomFeatureSearchState<IPresenceRoomFeature>
{
public:
    virtual bool AllowJoinFromPresence() const = 0;
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(4059745728, Redpoint::EOS::Rooms::Features, IPresenceRoomFeatureSearchState)
}

REDPOINT_EOS_CODE_GUARD_END()