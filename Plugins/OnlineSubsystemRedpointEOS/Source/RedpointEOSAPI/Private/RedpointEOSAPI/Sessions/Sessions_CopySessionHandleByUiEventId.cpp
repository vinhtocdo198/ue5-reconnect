// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Sessions/CopySessionHandleByUiEventId.h"

#include "RedpointEOSAPI/Templates/Impl/SyncReleasable.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Sessions
{

REDPOINT_EOSSDK_API_CALL_SYNC_RELEASABLE_IMPL(CopySessionHandleByUiEventId);

void FCopySessionHandleByUiEventId::MapOptions(
    NativeOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
    NativeOptions.UiEventId = *Options.UiEventId;
}

}

REDPOINT_EOS_CODE_GUARD_END()