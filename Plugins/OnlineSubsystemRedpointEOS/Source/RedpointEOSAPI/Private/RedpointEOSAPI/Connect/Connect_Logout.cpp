// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Connect/Logout.h"

#if EOS_VERSION_AT_LEAST(1, 16, 2)
#include "RedpointEOSAPI/Templates/Impl/Async.h"
#else
#include "RedpointEOSAPI/Templates/Impl/AsyncStub.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Connect
{

#if EOS_VERSION_AT_LEAST(1, 16, 2)

REDPOINT_EOSSDK_API_CALL_ASYNC_IMPL(Logout);

void FLogout::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.LocalUserId = *Options.LocalUserId;
}

void FLogout::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.ResultCode = NativeResult.ResultCode;
}

#else

REDPOINT_EOSSDK_API_CALL_ASYNC_STUB_IMPL(Logout);

void FLogout::MapResult(Result &Result, NativeConverter &Converter)
{
    Result.ResultCode = EOS_EResult::EOS_Success;
}

#endif

}

REDPOINT_EOS_CODE_GUARD_END()