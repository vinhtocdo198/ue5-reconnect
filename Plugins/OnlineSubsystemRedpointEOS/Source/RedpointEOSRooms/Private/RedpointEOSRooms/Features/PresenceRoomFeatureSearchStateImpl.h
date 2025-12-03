// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Features/PresenceRoomFeatureSearchState.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(459279400, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

class REDPOINTEOSROOMS_API FPresenceRoomFeatureSearchStateImpl : public IPresenceRoomFeatureSearchState
{
public:
    bool bAllowJoinFromPresence;

    FPresenceRoomFeatureSearchStateImpl();
    UE_NONCOPYABLE(FPresenceRoomFeatureSearchStateImpl);
    virtual ~FPresenceRoomFeatureSearchStateImpl() override = default;

    virtual bool AllowJoinFromPresence() const override;
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(459279400, Redpoint::EOS::Rooms::Features, FPresenceRoomFeatureSearchStateImpl)
}

REDPOINT_EOS_CODE_GUARD_END()