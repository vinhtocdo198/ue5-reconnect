// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Sessions/CopyActiveSessionHandle.h"

#include "RedpointEOSAPI/Templates/Impl/SyncReleasable.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Sessions
{

REDPOINT_EOSSDK_API_CALL_SYNC_RELEASABLE_IMPL(CopyActiveSessionHandle);

void FCopyActiveSessionHandle::MapOptions(
    NativeOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
    NativeOptions.SessionName = Allocator.AsAnsi(*Options.SessionName);
}

}

REDPOINT_EOS_CODE_GUARD_END()