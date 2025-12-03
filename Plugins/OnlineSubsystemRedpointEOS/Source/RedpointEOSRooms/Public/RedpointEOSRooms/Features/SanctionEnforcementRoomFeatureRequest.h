// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Features/RoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/SanctionEnforcementRoomFeature.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(482860834, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

class REDPOINTEOSROOMS_API FSanctionEnforcementRoomFeatureRequest
    : public IRoomFeatureRequest<ISanctionEnforcementRoomFeature>
{
public:
    FSanctionEnforcementRoomFeatureRequest() = default;
    UE_NONCOPYABLE(FSanctionEnforcementRoomFeatureRequest);
    virtual ~FSanctionEnforcementRoomFeatureRequest() override = default;
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(482860834, Redpoint::EOS::Rooms::Features, FSanctionEnforcementRoomFeatureRequest)
}

REDPOINT_EOS_CODE_GUARD_END()