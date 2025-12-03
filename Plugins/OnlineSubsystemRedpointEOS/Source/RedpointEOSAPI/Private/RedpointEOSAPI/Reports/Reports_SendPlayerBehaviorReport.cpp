// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Reports/SendPlayerBehaviorReport.h"

#include "RedpointEOSAPI/Templates/Impl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Reports
{

REDPOINT_EOSSDK_API_CALL_ASYNC_IMPL(SendPlayerBehaviorReport);

void FSendPlayerBehaviorReport::MapOptions(
    NativeOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
    NativeOptions.ReporterUserId = *Options.ReporterUserId;
    NativeOptions.ReportedUserId = *Options.ReportedUserId;
    NativeOptions.Category = *Options.Category;
    NativeOptions.Message = Allocator.AsUtf8(*Options.Message, Templates::Util::EAllocationFlags::ReturnNullptrIfEmpty);
    NativeOptions.Context = Allocator.AsUtf8(*Options.Context, Templates::Util::EAllocationFlags::ReturnNullptrIfEmpty);
}

void FSendPlayerBehaviorReport::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.ResultCode = NativeResult.ResultCode;
}

}

REDPOINT_EOS_CODE_GUARD_END()