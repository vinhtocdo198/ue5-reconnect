// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Features/SanctionEnforcementRoomFeatureImpl.h"

#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSRooms/Features/SanctionEnforcementRoomFeatureRequest.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(138738348, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

TSharedPtr<IRoomFeatureRequestBase> FSanctionEnforcementRoomFeatureImpl::GetFeatureRequestForUpdate() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return MakeShared<FSanctionEnforcementRoomFeatureRequest>();
}

}

REDPOINT_EOS_CODE_GUARD_END()