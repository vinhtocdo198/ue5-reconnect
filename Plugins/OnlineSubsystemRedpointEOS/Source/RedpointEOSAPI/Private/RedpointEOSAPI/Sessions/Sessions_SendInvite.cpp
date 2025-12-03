// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Sessions/SendInvite.h"

#include "RedpointEOSAPI/Templates/Impl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Sessions
{

REDPOINT_EOSSDK_API_CALL_ASYNC_IMPL(SendInvite);

void FSendInvite::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.LocalUserId = *Options.LocalUserId;
    NativeOptions.SessionName = Allocator.AsAnsi(*Options.SessionName);
    NativeOptions.TargetUserId = *Options.TargetUserId;
}

void FSendInvite::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.ResultCode = NativeResult.ResultCode;
}

}

REDPOINT_EOS_CODE_GUARD_END()