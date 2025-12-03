// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Presence/QueryPresence.h"

#include "RedpointEOSAPI/Templates/Impl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Presence
{

REDPOINT_EOSSDK_API_CALL_ASYNC_IMPL(QueryPresence);

void FQueryPresence::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.LocalUserId = *Options.LocalUserId;
    NativeOptions.TargetUserId = *Options.TargetUserId;
}

void FQueryPresence::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.ResultCode = NativeResult.ResultCode;
    Result.LocalUserId = NativeResult.LocalUserId;
    Result.TargetUserId = NativeResult.TargetUserId;
}

}

REDPOINT_EOS_CODE_GUARD_END()