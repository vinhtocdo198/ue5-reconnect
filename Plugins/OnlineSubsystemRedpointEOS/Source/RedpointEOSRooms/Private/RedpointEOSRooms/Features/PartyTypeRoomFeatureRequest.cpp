// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Features/PartyTypeRoomFeatureRequest.h"

#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSRooms/Features/PartyTypeRoomFeature.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2396472480, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

// @note: This is a copy of the value from OnlinePartyInterface.h, since we can't directly reference OSSv1 headers
// inside this module.
const uint32 PartyTypePrimaryPartyId = 0x11111111;

FPartyTypeRoomFeatureRequest::FPartyTypeRoomFeatureRequest(uint32 InPartyType)
    : PartyTypeIdValue(InPartyType)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);
}

bool FPartyTypeRoomFeatureRequest::IsValid() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return true;
}

uint32 FPartyTypeRoomFeatureRequest::PartyTypeId() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return this->PartyTypeIdValue;
}

}

REDPOINT_EOS_CODE_GUARD_END()