// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Features/BucketRoomFeature.h"
#include "RedpointEOSRooms/Features/RoomFeatureRequest.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(828848666, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

class REDPOINTEOSROOMS_API FBucketRoomFeatureRequest : public IRoomFeatureRequest<IBucketRoomFeature>
{
private:
    FString BucketValue;

public:
    FBucketRoomFeatureRequest(const FString &InBucket);
    UE_NONCOPYABLE(FBucketRoomFeatureRequest);
    virtual ~FBucketRoomFeatureRequest() override = default;

    virtual bool IsValid() const override;
    const FString &Bucket() const;
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(828848666, Redpoint::EOS::Rooms::Features, FBucketRoomFeatureRequest)
}

REDPOINT_EOS_CODE_GUARD_END()