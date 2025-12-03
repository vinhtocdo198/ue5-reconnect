// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Features/PublicAdvertisementRoomFeature.h"
#include "RedpointEOSRooms/Features/RoomFeatureRequest.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3010257269, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

class REDPOINTEOSROOMS_API FPublicAdvertisementRoomFeatureRequest
    : public IRoomFeatureRequest<IPublicAdvertisementRoomFeature>
{
public:
    FPublicAdvertisementRoomFeatureRequest() = default;
    UE_NONCOPYABLE(FPublicAdvertisementRoomFeatureRequest);
    virtual ~FPublicAdvertisementRoomFeatureRequest() override = default;
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(3010257269, Redpoint::EOS::Rooms::Features, FPublicAdvertisementRoomFeatureRequest)
}

REDPOINT_EOS_CODE_GUARD_END()