// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Features/RoomFeatureSearchState.h"
#include "RedpointEOSRooms/Features/ServerAddressRoomFeature.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(946483635, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

class REDPOINTEOSROOMS_API IServerAddressRoomFeatureSearchState
    : public IRoomFeatureSearchState<IServerAddressRoomFeature>
{
public:
    virtual const FString &Address() const = 0;
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(946483635, Redpoint::EOS::Rooms::Features, IServerAddressRoomFeatureSearchState)
}

REDPOINT_EOS_CODE_GUARD_END()