// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Sessions/UpdateSession.h"

#include "RedpointEOSAPI/Templates/Impl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Sessions
{

REDPOINT_EOSSDK_API_CALL_ASYNC_IMPL(UpdateSession);

void FUpdateSession::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.SessionModificationHandle = (*Options.SessionModificationHandle)->Value();
}

void FUpdateSession::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.ResultCode = NativeResult.ResultCode;
    Result.SessionName = Converter.FromAnsi(NativeResult.SessionName);
    Result.SessionId = Converter.FromAnsi(NativeResult.SessionId);
}

}

REDPOINT_EOS_CODE_GUARD_END()