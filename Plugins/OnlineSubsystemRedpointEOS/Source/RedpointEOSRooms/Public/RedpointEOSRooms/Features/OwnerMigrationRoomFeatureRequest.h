// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Features/OwnerMigrationRoomFeature.h"
#include "RedpointEOSRooms/Features/RoomFeatureRequest.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(521513833, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

class REDPOINTEOSROOMS_API FOwnerMigrationRoomFeatureRequest : public IRoomFeatureRequest<IOwnerMigrationRoomFeature>
{
public:
    FOwnerMigrationRoomFeatureRequest() = default;
    UE_NONCOPYABLE(FOwnerMigrationRoomFeatureRequest);
    virtual ~FOwnerMigrationRoomFeatureRequest() override = default;
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(521513833, Redpoint::EOS::Rooms::Features, FOwnerMigrationRoomFeatureRequest)
}

REDPOINT_EOS_CODE_GUARD_END()