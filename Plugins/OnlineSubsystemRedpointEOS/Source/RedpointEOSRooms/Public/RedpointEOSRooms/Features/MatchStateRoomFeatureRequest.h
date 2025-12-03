// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Features/MatchStateRoomFeature.h"
#include "RedpointEOSRooms/Features/RoomFeatureRequest.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3491375314, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

class REDPOINTEOSROOMS_API FMatchStateRoomFeatureRequest : public IRoomFeatureRequest<IMatchStateRoomFeature>
{
private:
    bool bAllowJoinInProgressValue;

public:
    FMatchStateRoomFeatureRequest(bool bInAllowJoinInProgress);
    UE_NONCOPYABLE(FMatchStateRoomFeatureRequest);
    virtual ~FMatchStateRoomFeatureRequest() override = default;

    bool AllowJoinInProgress() const;
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(3491375314, Redpoint::EOS::Rooms::Features, FMatchStateRoomFeatureRequest)
}

REDPOINT_EOS_CODE_GUARD_END()