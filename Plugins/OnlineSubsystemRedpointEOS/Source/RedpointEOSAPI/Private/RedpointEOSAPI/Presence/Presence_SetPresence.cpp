// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Presence/SetPresence.h"

#include "RedpointEOSAPI/Templates/Impl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Presence
{

REDPOINT_EOSSDK_API_CALL_ASYNC_IMPL(SetPresence);

void FSetPresence::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.LocalUserId = *Options.LocalUserId;
    NativeOptions.PresenceModificationHandle = (*Options.PresenceModificationHandle)->Value();
}

void FSetPresence::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.ResultCode = NativeResult.ResultCode;
    Result.LocalUserId = NativeResult.LocalUserId;
}

}

REDPOINT_EOS_CODE_GUARD_END()