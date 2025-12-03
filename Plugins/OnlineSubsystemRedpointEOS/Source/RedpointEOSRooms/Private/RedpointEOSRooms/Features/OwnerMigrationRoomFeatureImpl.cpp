// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Features/OwnerMigrationRoomFeatureImpl.h"

#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSRooms/Features/OwnerMigrationRoomFeatureRequest.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2179814654, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

TSharedPtr<IRoomFeatureRequestBase> FOwnerMigrationRoomFeatureImpl::GetFeatureRequestForUpdate() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return MakeShared<FOwnerMigrationRoomFeatureRequest>();
}

}

REDPOINT_EOS_CODE_GUARD_END()