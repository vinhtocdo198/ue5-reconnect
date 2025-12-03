// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Features/BucketRoomFeature.h"
#include "RedpointEOSRooms/Features/RoomFeatureSearchState.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(557706760, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

class REDPOINTEOSROOMS_API IBucketRoomFeatureSearchState : public IRoomFeatureSearchState<IBucketRoomFeature>
{
public:
    virtual const FString &Bucket() const = 0;
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(557706760, Redpoint::EOS::Rooms::Features, IBucketRoomFeatureSearchState)
}

REDPOINT_EOS_CODE_GUARD_END()