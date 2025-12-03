// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Features/BucketRoomFeatureRequest.h"

#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSRooms/Features/BucketRoomFeature.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(828848666, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

FBucketRoomFeatureRequest::FBucketRoomFeatureRequest(const FString &InBucket)
    : BucketValue(InBucket)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);
}

bool FBucketRoomFeatureRequest::IsValid() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return !this->BucketValue.IsEmpty();
}

const FString &FBucketRoomFeatureRequest::Bucket() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return this->BucketValue;
}

}

REDPOINT_EOS_CODE_GUARD_END()