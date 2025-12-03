// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Features/MatchStateRoomFeatureRequest.h"

#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSRooms/Features/MatchStateRoomFeature.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3491375314, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

FMatchStateRoomFeatureRequest::FMatchStateRoomFeatureRequest(bool bInAllowJoinInProgress)
    : bAllowJoinInProgressValue(bInAllowJoinInProgress)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);
}

bool FMatchStateRoomFeatureRequest::AllowJoinInProgress() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return this->bAllowJoinInProgressValue;
}

}

REDPOINT_EOS_CODE_GUARD_END()