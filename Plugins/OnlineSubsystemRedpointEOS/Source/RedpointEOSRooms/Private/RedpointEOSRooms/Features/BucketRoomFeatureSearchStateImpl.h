// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Features/BucketRoomFeatureSearchState.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1692279607, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

class REDPOINTEOSROOMS_API FBucketRoomFeatureSearchStateImpl : public IBucketRoomFeatureSearchState
{
public:
    FString BucketValue;

    FBucketRoomFeatureSearchStateImpl();
    UE_NONCOPYABLE(FBucketRoomFeatureSearchStateImpl);
    virtual ~FBucketRoomFeatureSearchStateImpl() override = default;

    virtual const FString &Bucket() const override;
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(1692279607, Redpoint::EOS::Rooms::Features, FBucketRoomFeatureSearchStateImpl)
}

REDPOINT_EOS_CODE_GUARD_END()