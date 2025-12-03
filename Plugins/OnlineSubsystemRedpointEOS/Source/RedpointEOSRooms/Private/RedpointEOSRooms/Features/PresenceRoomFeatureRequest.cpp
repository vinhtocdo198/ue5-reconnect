// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Features/PresenceRoomFeatureRequest.h"

#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSRooms/Features/PresenceRoomFeature.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2629249321, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

FPresenceRoomFeatureRequest::FPresenceRoomFeatureRequest(bool bAllowJoinByPresence)
    : bAllowJoinByPresenceValue(bAllowJoinByPresence)
{
}

bool FPresenceRoomFeatureRequest::AllowJoinByPresence() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return this->bAllowJoinByPresenceValue;
}

}

REDPOINT_EOS_CODE_GUARD_END()