// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Features/PublicMemberListRoomFeature.h"

#include "RedpointEOSAPI/Memory.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(621398794, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

FName IPublicMemberListRoomFeature::GetFeatureName()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return FName(TEXT("PublicMemberList"));
}

}

REDPOINT_EOS_CODE_GUARD_END()