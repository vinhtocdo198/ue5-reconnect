// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/PlayerDataStorage/DuplicateFile.h"

#include "RedpointEOSAPI/Templates/Impl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::PlayerDataStorage
{

REDPOINT_EOSSDK_API_CALL_ASYNC_IMPL(DuplicateFile);

void FDuplicateFile::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.LocalUserId = *Options.LocalUserId;
    NativeOptions.SourceFilename = Allocator.AsUtf8(*Options.SourceFilename);
    NativeOptions.DestinationFilename = Allocator.AsUtf8(*Options.DestinationFilename);
}

void FDuplicateFile::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.ResultCode = NativeResult.ResultCode;
    Result.LocalUserId = NativeResult.LocalUserId;
}

}

REDPOINT_EOS_CODE_GUARD_END()