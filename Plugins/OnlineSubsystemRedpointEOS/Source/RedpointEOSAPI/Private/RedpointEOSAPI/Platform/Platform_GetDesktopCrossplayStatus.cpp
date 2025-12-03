// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Platform/GetDesktopCrossplayStatus.h"

#include "RedpointEOSAPI/Templates/Impl/SyncCopyNoAlloc.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Platform
{

REDPOINT_EOSSDK_API_CALL_SYNC_COPY_NOALLOC_IMPL(GetDesktopCrossplayStatus);

void FGetDesktopCrossplayStatus::MapOptions(
    NativeOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
}

void FGetDesktopCrossplayStatus::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.Status = NativeResult.Status;
    Result.ServiceInitResult = NativeResult.ServiceInitResult;
}

}

REDPOINT_EOS_CODE_GUARD_END()