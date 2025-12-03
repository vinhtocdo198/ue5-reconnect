// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Connect/CreateUser.h"

#include "RedpointEOSAPI/Templates/Impl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Connect
{

REDPOINT_EOSSDK_API_CALL_ASYNC_IMPL(CreateUser);

void FCreateUser::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.ContinuanceToken = *Options.ContinuanceToken;
}

void FCreateUser::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.ResultCode = NativeResult.ResultCode;
    Result.LocalUserId = NativeResult.LocalUserId;
}

}

REDPOINT_EOS_CODE_GUARD_END()