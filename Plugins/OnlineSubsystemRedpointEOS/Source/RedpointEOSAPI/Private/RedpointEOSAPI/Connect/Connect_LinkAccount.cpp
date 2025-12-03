// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Connect/LinkAccount.h"

#include "RedpointEOSAPI/Templates/Impl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Connect
{

REDPOINT_EOSSDK_API_CALL_ASYNC_IMPL(LinkAccount);

void FLinkAccount::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.ContinuanceToken = *Options.ContinuanceToken;
    NativeOptions.LocalUserId = *Options.LocalUserId;
}

void FLinkAccount::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.ResultCode = NativeResult.ResultCode;
    Result.LocalUserId = NativeResult.LocalUserId;
}

}

REDPOINT_EOS_CODE_GUARD_END()