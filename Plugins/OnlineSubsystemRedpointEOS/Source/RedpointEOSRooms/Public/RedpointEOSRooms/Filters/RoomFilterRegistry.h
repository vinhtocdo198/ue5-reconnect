// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Utils/GenericRegistry.h"
#include "RedpointEOSRooms/Filters/RoomFilter.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(4104035497, Redpoint::EOS::Rooms::Filters)
{
using namespace ::Redpoint::EOS::Rooms::Filters;
using namespace ::Redpoint::EOS::Core::Utils;

class REDPOINTEOSROOMS_API FRoomFilterRegistry : public TGenericRegistry<IRoomFilter, FRoomFilterRegistry>
{
    REDPOINT_EOS_DECLARE_GENERIC_REGISTRY_BODY()
};

}

namespace Redpoint::EOS::Rooms::Filters
{
REDPOINT_EOS_FILE_NS_EXPORT(4104035497, Redpoint::EOS::Rooms::Filters, FRoomFilterRegistry)
}

REDPOINT_EOS_CODE_GUARD_END()