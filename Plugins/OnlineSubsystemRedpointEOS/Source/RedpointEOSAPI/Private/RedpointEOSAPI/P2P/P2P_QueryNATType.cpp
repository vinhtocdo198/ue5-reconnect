// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/P2P/QueryNATType.h"

#if WITH_CLIENT_CODE
#include "RedpointEOSAPI/Templates/Impl/Async.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if WITH_CLIENT_CODE

namespace Redpoint::EOS::API::P2P
{

REDPOINT_EOSSDK_API_CALL_ASYNC_IMPL(QueryNATType);

void FQueryNATType::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
}

void FQueryNATType::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.ResultCode = NativeResult.ResultCode;
    Result.NATType = NativeResult.NATType;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()