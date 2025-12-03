// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/SessionModification/SetAllowedPlatformIds.h"

#include "RedpointEOSAPI/Templates/Impl/ObjectSync.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::SessionModification
{

REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_IMPL(SetAllowedPlatformIds);

void FSetAllowedPlatformIds::MapOptions(
    NativeOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
    NativeOptions.AllowedPlatformIds = reinterpret_cast<const uint32_t *>(
        Allocator.AllocateArray(*Options.AllowedPlatformIds, NativeOptions.AllowedPlatformIdsCount));
}

}

REDPOINT_EOS_CODE_GUARD_END()