// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Features/PublicAdvertisementRoomFeatureImpl.h"

#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSRooms/Features/PublicAdvertisementRoomFeatureRequest.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1095917875, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

TSharedPtr<IRoomFeatureRequestBase> FPublicAdvertisementRoomFeatureImpl::GetFeatureRequestForUpdate() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return MakeShared<FPublicAdvertisementRoomFeatureRequest>();
}

}

REDPOINT_EOS_CODE_GUARD_END()