// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Features/AliasRoomFeatureSearchState.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3585178210, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

class REDPOINTEOSROOMS_API FAliasRoomFeatureSearchStateImpl : public IAliasRoomFeatureSearchState
{
public:
    FAliasRoomFeatureSearchStateImpl() = default;
    UE_NONCOPYABLE(FAliasRoomFeatureSearchStateImpl);
    virtual ~FAliasRoomFeatureSearchStateImpl() override = default;
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(3585178210, Redpoint::EOS::Rooms::Features, FAliasRoomFeatureSearchStateImpl)
}

REDPOINT_EOS_CODE_GUARD_END()