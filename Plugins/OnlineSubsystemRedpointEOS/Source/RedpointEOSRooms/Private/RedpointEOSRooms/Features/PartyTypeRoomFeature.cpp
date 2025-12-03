// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Features/PartyTypeRoomFeature.h"

#include "RedpointEOSAPI/Memory.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3038231154, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

FName IPartyTypeRoomFeature::GetFeatureName()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return FName(TEXT("PartyType"));
}

}

REDPOINT_EOS_CODE_GUARD_END()