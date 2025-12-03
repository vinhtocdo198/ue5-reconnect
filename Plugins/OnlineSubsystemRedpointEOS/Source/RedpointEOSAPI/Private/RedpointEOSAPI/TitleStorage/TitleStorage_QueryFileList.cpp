// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/TitleStorage/QueryFileList.h"

#include "RedpointEOSAPI/Templates/Impl/AsyncListCopy.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::TitleStorage
{

REDPOINT_EOSSDK_API_CALL_ASYNC_LIST_COPY_IMPL(QueryFileList);

void FQueryFileList::MapQueryOptions(
    NativeQueryOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
    NativeOptions.LocalUserId = *Options.LocalUserId;

    TArray<const char *> ListOfTagsConstChar;
    for (const auto &Entry : *Options.ListOfTags)
    {
        ListOfTagsConstChar.Add(Allocator.AsUtf8(Entry, Templates::Util::EAllocationFlags::Default));
    }
    NativeOptions.ListOfTags = const_cast<const char **>(Allocator.AllocateArray(
        ListOfTagsConstChar,
        NativeOptions.ListOfTagsCount,
        Templates::Util::EAllocationFlags::Default));
}

void FQueryFileList::MapCountOptions(
    NativeCountOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
    NativeOptions.LocalUserId = *Options.LocalUserId;
}

void FQueryFileList::MapCopyByIndexOptions(
    NativeCopyByIndexOptions &NativeOptions,
    const Options &Options,
    uint32_t Index,
    NativeAllocator &Allocator)
{
    NativeOptions.LocalUserId = *Options.LocalUserId;
    NativeOptions.Index = Index;
}

void FQueryFileList::MapCopyByIndexResult(
    ResultEntry &Result,
    const NativeCopyByIndexResult &NativeResult,
    NativeConverter &Converter)
{
    Result.Filename = Converter.FromUtf8(NativeResult.Filename);
    Result.FileSizeBytes = NativeResult.FileSizeBytes;
    Result.MD5Hash = Converter.FromUtf8(NativeResult.MD5Hash);
    Result.UnencryptedDataSizeBytes = NativeResult.UnencryptedDataSizeBytes;
}

}

REDPOINT_EOS_CODE_GUARD_END()