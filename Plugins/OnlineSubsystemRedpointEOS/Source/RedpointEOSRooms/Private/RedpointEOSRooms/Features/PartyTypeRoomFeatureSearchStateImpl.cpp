// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Features/PartyTypeRoomFeatureSearchStateImpl.h"

#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSRooms/Features/PartyTypeRoomFeatureRequest.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2289569929, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

FPartyTypeRoomFeatureSearchStateImpl::FPartyTypeRoomFeatureSearchStateImpl()
    : PartyTypeIdValue()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);
}

uint32 FPartyTypeRoomFeatureSearchStateImpl::PartyTypeId() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return this->PartyTypeIdValue;
}

bool FPartyTypeRoomFeatureSearchStateImpl::PartyTypeIndicatesPresenceEnabled() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return this->PartyTypeIdValue == Redpoint::EOS::Rooms::Features::PartyTypePrimaryPartyId;
}

}

REDPOINT_EOS_CODE_GUARD_END()