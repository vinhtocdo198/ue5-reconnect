// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Features/RoomFeatureRequest.h"
#include "RedpointEOSRooms/RoomAttribute.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1648834184, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

class REDPOINTEOSROOMS_API IRoomFeature
{
public:
    IRoomFeature() = default;
    IRoomFeature(const IRoomFeature &) = default;
    IRoomFeature &operator=(const IRoomFeature &) = default;
    IRoomFeature &operator=(IRoomFeature &&) = default;
    virtual ~IRoomFeature() = default;

    virtual TSharedPtr<IRoomFeatureRequestBase> GetFeatureRequestForUpdate() const = 0;
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(1648834184, Redpoint::EOS::Rooms::Features, IRoomFeature)
}

REDPOINT_EOS_CODE_GUARD_END()