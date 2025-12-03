// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Features/ServerAddressRoomFeatureImpl.h"

#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSRooms/Features/ServerAddressRoomFeatureRequest.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1983455148, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

TSharedPtr<IRoomFeatureRequestBase> FServerAddressRoomFeatureImpl::GetFeatureRequestForUpdate() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    checkf(
        false,
        TEXT("FServerAddressRoomFeatureImpl::GetFeatureRequestForUpdate is not implemented. It needs a callback for "
             "the room provider to figure it out from room state."));
    return MakeShared<FServerAddressRoomFeatureRequest>(
        FServerAddressRoomFeatureRequest::EServerAddressMode::Manual,
        TEXT(""));
}

}

REDPOINT_EOS_CODE_GUARD_END()