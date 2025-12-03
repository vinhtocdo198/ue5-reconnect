// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Auth/DeletePersistentAuth.h"

#include "RedpointEOSAPI/Templates/Impl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Auth
{

REDPOINT_EOSSDK_API_CALL_ASYNC_IMPL(DeletePersistentAuth);

void FDeletePersistentAuth::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.RefreshToken =
        Allocator.AsAnsi(Options.RefreshToken, Templates::Util::EAllocationFlags::ReturnNullptrIfEmpty);
}

void FDeletePersistentAuth::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.ResultCode = NativeResult.ResultCode;
}

}

REDPOINT_EOS_CODE_GUARD_END()