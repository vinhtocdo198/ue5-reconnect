// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/LobbySearch/SetTargetUserId.h"

#include "RedpointEOSAPI/Templates/Impl/ObjectSync.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::LobbySearch
{

REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_IMPL(SetTargetUserId);

void FSetTargetUserId::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.TargetUserId = *Options.TargetUserId;
}

}

REDPOINT_EOS_CODE_GUARD_END()