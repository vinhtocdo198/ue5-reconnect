// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Features/PartyConfigurationRoomFeature.h"

#include "RedpointEOSAPI/Memory.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3188680639, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

FName IPartyConfigurationRoomFeature::GetFeatureName()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return FName(TEXT("PartyConfiguration"));
}

}

REDPOINT_EOS_CODE_GUARD_END()