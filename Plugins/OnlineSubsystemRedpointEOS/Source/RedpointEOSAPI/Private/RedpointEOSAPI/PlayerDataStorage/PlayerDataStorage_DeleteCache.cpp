// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/PlayerDataStorage/DeleteCache.h"

#include "RedpointEOSAPI/Templates/Impl/AsyncWithImmediateError.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::PlayerDataStorage
{

REDPOINT_EOSSDK_API_CALL_ASYNC_WITH_IMMEDIATE_ERROR_IMPL(DeleteCache);

void FDeleteCache::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.LocalUserId = *Options.LocalUserId;
}

void FDeleteCache::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.ResultCode = NativeResult.ResultCode;
}

}

REDPOINT_EOS_CODE_GUARD_END()