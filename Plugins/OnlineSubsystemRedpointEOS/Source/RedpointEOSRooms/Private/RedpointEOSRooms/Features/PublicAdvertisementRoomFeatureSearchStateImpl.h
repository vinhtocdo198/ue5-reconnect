// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Features/PublicAdvertisementRoomFeatureSearchState.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1058619315, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

class REDPOINTEOSROOMS_API FPublicAdvertisementRoomFeatureSearchStateImpl
    : public IPublicAdvertisementRoomFeatureSearchState
{
public:
    FPublicAdvertisementRoomFeatureSearchStateImpl() = default;
    UE_NONCOPYABLE(FPublicAdvertisementRoomFeatureSearchStateImpl);
    virtual ~FPublicAdvertisementRoomFeatureSearchStateImpl() override = default;
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(1058619315, Redpoint::EOS::Rooms::Features, FPublicAdvertisementRoomFeatureSearchStateImpl)
}

REDPOINT_EOS_CODE_GUARD_END()