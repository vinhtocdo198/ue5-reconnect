// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Features/VoiceChatRoomFeatureSearchState.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3692167641, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

class REDPOINTEOSROOMS_API FVoiceChatRoomFeatureSearchStateImpl : public IVoiceChatRoomFeatureSearchState
{
public:
    FVoiceChatRoomFeatureSearchStateImpl() = default;
    UE_NONCOPYABLE(FVoiceChatRoomFeatureSearchStateImpl);
    virtual ~FVoiceChatRoomFeatureSearchStateImpl() override = default;
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(3692167641, Redpoint::EOS::Rooms::Features, FVoiceChatRoomFeatureSearchStateImpl)
}

REDPOINT_EOS_CODE_GUARD_END()