// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Features/BucketRoomFeatureImpl.h"

#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSRooms/Features/BucketRoomFeatureRequest.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2649860660, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

FString FBucketRoomFeatureImpl::GetBucketId() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return this->BucketId;
}

TSharedPtr<IRoomFeatureRequestBase> FBucketRoomFeatureImpl::GetFeatureRequestForUpdate() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return MakeShared<FBucketRoomFeatureRequest>(this->BucketId);
}

}

REDPOINT_EOS_CODE_GUARD_END()