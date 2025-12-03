// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Features/InvitesRoomFeature.h"
#include "RedpointEOSRooms/Features/RoomFeatureSearchState.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2187907719, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

class REDPOINTEOSROOMS_API IInvitesRoomFeatureSearchState : public IRoomFeatureSearchState<IInvitesRoomFeature>
{
public:
    virtual bool RejoinAfterKickRequiresInvite() const = 0;
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(2187907719, Redpoint::EOS::Rooms::Features, IInvitesRoomFeatureSearchState)
}

REDPOINT_EOS_CODE_GUARD_END()