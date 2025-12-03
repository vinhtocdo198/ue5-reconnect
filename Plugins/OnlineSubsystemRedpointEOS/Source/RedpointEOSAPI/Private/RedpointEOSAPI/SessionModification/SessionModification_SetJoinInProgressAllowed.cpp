// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/SessionModification/SetJoinInProgressAllowed.h"

#include "RedpointEOSAPI/Templates/Impl/ObjectSync.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::SessionModification
{

REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_IMPL(SetJoinInProgressAllowed);

void FSetJoinInProgressAllowed::MapOptions(
    NativeOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
    NativeOptions.bAllowJoinInProgress = *Options.bAllowJoinInProgress;
}

}

REDPOINT_EOS_CODE_GUARD_END()