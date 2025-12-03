// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Features/JoinByIdRoomFeature.h"
#include "RedpointEOSRooms/Features/RoomFeatureRequest.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2263683255, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

class REDPOINTEOSROOMS_API FJoinByIdRoomFeatureRequest : public IRoomFeatureRequest<IJoinByIdRoomFeature>
{
public:
    FJoinByIdRoomFeatureRequest() = default;
    UE_NONCOPYABLE(FJoinByIdRoomFeatureRequest);
    virtual ~FJoinByIdRoomFeatureRequest() override = default;
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(2263683255, Redpoint::EOS::Rooms::Features, FJoinByIdRoomFeatureRequest)
}

REDPOINT_EOS_CODE_GUARD_END()