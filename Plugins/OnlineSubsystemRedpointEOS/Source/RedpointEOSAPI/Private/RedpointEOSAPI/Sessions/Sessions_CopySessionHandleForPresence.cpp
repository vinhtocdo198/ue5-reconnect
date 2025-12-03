// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Sessions/CopySessionHandleForPresence.h"

#include "RedpointEOSAPI/Templates/Impl/SyncReleasable.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Sessions
{

REDPOINT_EOSSDK_API_CALL_SYNC_RELEASABLE_IMPL(CopySessionHandleForPresence);

void FCopySessionHandleForPresence::MapOptions(
    NativeOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
    NativeOptions.LocalUserId = *Options.LocalUserId;
}

}

REDPOINT_EOS_CODE_GUARD_END()