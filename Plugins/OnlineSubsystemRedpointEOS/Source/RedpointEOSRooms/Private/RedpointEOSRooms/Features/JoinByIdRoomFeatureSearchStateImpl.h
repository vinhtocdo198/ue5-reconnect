// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Features/JoinByIdRoomFeatureSearchState.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2546660827, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

class REDPOINTEOSROOMS_API FJoinByIdRoomFeatureSearchStateImpl : public IJoinByIdRoomFeatureSearchState
{
public:
    FJoinByIdRoomFeatureSearchStateImpl() = default;
    UE_NONCOPYABLE(FJoinByIdRoomFeatureSearchStateImpl);
    virtual ~FJoinByIdRoomFeatureSearchStateImpl() override = default;
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(2546660827, Redpoint::EOS::Rooms::Features, FJoinByIdRoomFeatureSearchStateImpl)
}

REDPOINT_EOS_CODE_GUARD_END()