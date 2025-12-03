// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Features/OwnerMigrationRoomFeatureSearchState.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2394076558, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

class REDPOINTEOSROOMS_API FOwnerMigrationRoomFeatureSearchStateImpl : public IOwnerMigrationRoomFeatureSearchState
{
public:
    FOwnerMigrationRoomFeatureSearchStateImpl() = default;
    UE_NONCOPYABLE(FOwnerMigrationRoomFeatureSearchStateImpl);
    virtual ~FOwnerMigrationRoomFeatureSearchStateImpl() override = default;
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(2394076558, Redpoint::EOS::Rooms::Features, FOwnerMigrationRoomFeatureSearchStateImpl)
}

REDPOINT_EOS_CODE_GUARD_END()