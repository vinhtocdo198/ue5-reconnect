// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/PlayerDataStorage/DeleteFile.h"

#include "RedpointEOSAPI/Templates/Impl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::PlayerDataStorage
{

REDPOINT_EOSSDK_API_CALL_ASYNC_IMPL(DeleteFile);

void FDeleteFile::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.LocalUserId = *Options.LocalUserId;
    NativeOptions.Filename = Allocator.AsUtf8(*Options.Filename);
}

void FDeleteFile::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.ResultCode = NativeResult.ResultCode;
    Result.LocalUserId = NativeResult.LocalUserId;
}

}

REDPOINT_EOS_CODE_GUARD_END()