// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Sessions/JoinSession.h"

#include "RedpointEOSAPI/Templates/Impl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Sessions
{

REDPOINT_EOSSDK_API_CALL_ASYNC_IMPL(JoinSession);

void FJoinSession::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.SessionName = Allocator.AsAnsi(*Options.SessionName);
    NativeOptions.SessionHandle = (*Options.SessionHandle)->Value();
    NativeOptions.LocalUserId = *Options.LocalUserId;
    NativeOptions.bPresenceEnabled = *Options.bPresenceEnabled ? EOS_TRUE : EOS_FALSE;
}

void FJoinSession::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.ResultCode = NativeResult.ResultCode;
}

}

REDPOINT_EOS_CODE_GUARD_END()