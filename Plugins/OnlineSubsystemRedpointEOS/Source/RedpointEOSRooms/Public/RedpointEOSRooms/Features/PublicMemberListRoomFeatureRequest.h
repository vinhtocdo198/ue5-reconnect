// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Features/PublicMemberListRoomFeature.h"
#include "RedpointEOSRooms/Features/RoomFeatureRequest.h"
#include "RedpointEOSRooms/RoomUserId.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1891229547, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

class REDPOINTEOSROOMS_API FPublicMemberListRoomFeatureRequest
    : public IRoomFeatureRequest<IPublicMemberListRoomFeature>
{
public:
    FPublicMemberListRoomFeatureRequest() = default;
    UE_NONCOPYABLE(FPublicMemberListRoomFeatureRequest);
    virtual ~FPublicMemberListRoomFeatureRequest() override = default;
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(1891229547, Redpoint::EOS::Rooms::Features, FPublicMemberListRoomFeatureRequest)
}

REDPOINT_EOS_CODE_GUARD_END()