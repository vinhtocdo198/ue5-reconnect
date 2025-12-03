// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/PlayerDataStorage/QueryFile.h"

#include "RedpointEOSAPI/Templates/Impl/AsyncListCopyErrorableCount.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::PlayerDataStorage
{

REDPOINT_EOSSDK_API_CALL_ASYNC_LIST_COPY_ERRORABLE_COUNT_IMPL(QueryFile);

void FQueryFile::MapQueryOptions(NativeQueryOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.LocalUserId = *Options.LocalUserId;
    NativeOptions.Filename = Allocator.AsUtf8(*Options.Filename);
}

void FQueryFile::MapCountOptions(NativeCountOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.LocalUserId = *Options.LocalUserId;
}

void FQueryFile::MapCopyByIndexOptions(
    NativeCopyByIndexOptions &NativeOptions,
    const Options &Options,
    uint32_t Index,
    NativeAllocator &Allocator)
{
    NativeOptions.LocalUserId = *Options.LocalUserId;
    NativeOptions.Index = Index;
}

void FQueryFile::MapCopyByIndexResult(
    ResultEntry &Result,
    const NativeCopyByIndexResult &NativeResult,
    NativeConverter &Converter)
{
    Result.Filename = Converter.FromUtf8(NativeResult.Filename);
    Result.FileSizeBytes = NativeResult.FileSizeBytes;
    Result.LastModifiedTime = NativeResult.LastModifiedTime == EOS_PLAYERDATASTORAGE_TIME_UNDEFINED
                                  ? TOptional<FDateTime>()
                                  : FDateTime::FromUnixTimestamp(NativeResult.LastModifiedTime);
    Result.MD5Hash = Converter.FromUtf8(NativeResult.MD5Hash);
    Result.UnencryptedDataSizeBytes = NativeResult.UnencryptedDataSizeBytes;
}

}

REDPOINT_EOS_CODE_GUARD_END()