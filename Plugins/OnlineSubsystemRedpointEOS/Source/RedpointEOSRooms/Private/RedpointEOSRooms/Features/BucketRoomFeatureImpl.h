// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSRooms/Features/BucketRoomFeature.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2649860660, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

class FBucketRoomFeatureImpl : public IBucketRoomFeature
{
public:
    FString BucketId;

    virtual FString GetBucketId() const override;
    virtual TSharedPtr<IRoomFeatureRequestBase> GetFeatureRequestForUpdate() const override;
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(2649860660, Redpoint::EOS::Rooms::Features, FBucketRoomFeatureImpl)
}

REDPOINT_EOS_CODE_GUARD_END()