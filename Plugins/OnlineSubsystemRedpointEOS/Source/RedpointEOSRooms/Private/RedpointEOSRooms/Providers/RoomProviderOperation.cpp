// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Providers/RoomProviderOperation.h"

#include "RedpointEOSAPI/Memory.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1191434334, Redpoint::EOS::Rooms::Providers)
{
using namespace ::Redpoint::EOS::Rooms::Providers;

#if !NO_LOGGING

TSharedRef<const FJsonObject> IRoomProviderOperation::GetDebugJson()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->GetMutableDebugJson();
}

#endif

}

REDPOINT_EOS_CODE_GUARD_END()