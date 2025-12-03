// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Features/PresenceRoomFeatureSearchStateImpl.h"

#include "RedpointEOSAPI/Memory.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(459279400, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

FPresenceRoomFeatureSearchStateImpl::FPresenceRoomFeatureSearchStateImpl()
    : bAllowJoinFromPresence()
{
}

bool FPresenceRoomFeatureSearchStateImpl::AllowJoinFromPresence() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return this->bAllowJoinFromPresence;
}

}

REDPOINT_EOS_CODE_GUARD_END()