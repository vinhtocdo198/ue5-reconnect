// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Features/RoomFeature.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3038231154, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

class REDPOINTEOSROOMS_API IPartyTypeRoomFeature : public IRoomFeature
{
public:
    static FName GetFeatureName();

    virtual uint32 GetPartyTypeId() const = 0;
    virtual bool GetPartyTypeIndicatesPresenceEnabled() const = 0;
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(3038231154, Redpoint::EOS::Rooms::Features, IPartyTypeRoomFeature)
}

REDPOINT_EOS_CODE_GUARD_END()