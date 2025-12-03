// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Features/RoomFeatureSearchState.h"
#include "RedpointEOSRooms/Features/VoiceChatRoomFeature.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1319252759, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

class REDPOINTEOSROOMS_API IVoiceChatRoomFeatureSearchState : public IRoomFeatureSearchState<IVoiceChatRoomFeature>
{
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(1319252759, Redpoint::EOS::Rooms::Features, IVoiceChatRoomFeatureSearchState)
}

REDPOINT_EOS_CODE_GUARD_END()