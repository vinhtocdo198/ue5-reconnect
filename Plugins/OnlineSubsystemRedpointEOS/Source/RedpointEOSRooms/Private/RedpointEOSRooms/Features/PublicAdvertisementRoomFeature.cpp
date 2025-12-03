// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Features/PublicAdvertisementRoomFeature.h"

#include "RedpointEOSAPI/Memory.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1053461017, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

FName IPublicAdvertisementRoomFeature::GetFeatureName()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return FName(TEXT("PublicAdvertisement"));
}

}

REDPOINT_EOS_CODE_GUARD_END()