// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Connect/QueryProductUserIdMappings.h"

#include "RedpointEOSAPI/Templates/Impl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Connect
{

REDPOINT_EOSSDK_API_CALL_ASYNC_IMPL(QueryProductUserIdMappings);

void FQueryProductUserIdMappings::MapOptions(
    NativeOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
    NativeOptions.LocalUserId = *Options.LocalUserId;
    NativeOptions.ProductUserIds = const_cast<EOS_ProductUserId *>(
        Allocator.AllocateArray(*Options.ProductUserIds, NativeOptions.ProductUserIdCount));
}

void FQueryProductUserIdMappings::MapResult(
    Result &Result,
    const NativeResult &NativeResult,
    NativeConverter &Converter)
{
    Result.ResultCode = NativeResult.ResultCode;
    Result.LocalUserId = NativeResult.LocalUserId;
}

}

REDPOINT_EOS_CODE_GUARD_END()