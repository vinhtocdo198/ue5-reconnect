// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Features/AliasRoomFeature.h"

#include "RedpointEOSAPI/Memory.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(623942501, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

FName IAliasRoomFeature::GetFeatureName()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return FName(TEXT("Alias"));
}

}

REDPOINT_EOS_CODE_GUARD_END()