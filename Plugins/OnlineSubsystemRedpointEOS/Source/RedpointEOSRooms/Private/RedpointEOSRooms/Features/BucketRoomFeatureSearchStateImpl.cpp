// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Features/BucketRoomFeatureSearchStateImpl.h"

#include "RedpointEOSAPI/Memory.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1692279607, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

FBucketRoomFeatureSearchStateImpl::FBucketRoomFeatureSearchStateImpl()
    : BucketValue()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);
}

const FString &FBucketRoomFeatureSearchStateImpl::Bucket() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return this->BucketValue;
}

}

REDPOINT_EOS_CODE_GUARD_END()