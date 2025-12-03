// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Features/RoomFeature.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(4085206016, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

class REDPOINTEOSROOMS_API IPresenceRoomFeature : public IRoomFeature
{
public:
    static FName GetFeatureName();

    virtual bool AllowJoinFromPresence() const = 0;
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(4085206016, Redpoint::EOS::Rooms::Features, IPresenceRoomFeature)
}

REDPOINT_EOS_CODE_GUARD_END()