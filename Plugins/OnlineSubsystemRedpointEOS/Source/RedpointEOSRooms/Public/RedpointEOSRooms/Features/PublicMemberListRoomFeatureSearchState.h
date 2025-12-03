// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Features/PublicMemberListRoomFeature.h"
#include "RedpointEOSRooms/Features/RoomFeatureSearchState.h"
#include "RedpointEOSRooms/RoomUserId.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(838690609, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Features;

class REDPOINTEOSROOMS_API IPublicMemberListRoomFeatureSearchState
    : public IRoomFeatureSearchState<IPublicMemberListRoomFeature>
{
public:
    virtual uint32 MaxMembers() const = 0;
    virtual uint32 AvailableSlots() const = 0;
    virtual const TArray<FRoomUserId> &CurrentMembers() const = 0;
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(838690609, Redpoint::EOS::Rooms::Features, IPublicMemberListRoomFeatureSearchState)
}

REDPOINT_EOS_CODE_GUARD_END()