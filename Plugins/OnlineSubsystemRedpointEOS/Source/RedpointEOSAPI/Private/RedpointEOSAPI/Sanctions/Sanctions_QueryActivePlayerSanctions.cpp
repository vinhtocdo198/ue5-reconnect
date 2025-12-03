// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Sanctions/QueryActivePlayerSanctions.h"

#include "RedpointEOSAPI/Templates/Impl/AsyncListCopy.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Sanctions
{

REDPOINT_EOSSDK_API_CALL_ASYNC_LIST_COPY_IMPL(QueryActivePlayerSanctions);

void FQueryActivePlayerSanctions::MapQueryOptions(
    NativeQueryOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
    NativeOptions.TargetUserId = *Options.TargetUserId;
    NativeOptions.LocalUserId = *Options.LocalUserId;
}

void FQueryActivePlayerSanctions::MapCountOptions(
    NativeCountOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
    NativeOptions.TargetUserId = *Options.TargetUserId;
}

void FQueryActivePlayerSanctions::MapCopyByIndexOptions(
    NativeCopyByIndexOptions &NativeOptions,
    const Options &Options,
    uint32_t Index,
    NativeAllocator &Allocator)
{
    NativeOptions.TargetUserId = *Options.TargetUserId;
    NativeOptions.SanctionIndex = Index;
}

void FQueryActivePlayerSanctions::MapCopyByIndexResult(
    ResultEntry &Result,
    const NativeCopyByIndexResult &NativeResult,
    NativeConverter &Converter)
{
    Result.TimePlaced = FDateTime::FromUnixTimestamp(NativeResult.TimePlaced);
    Result.Action = Converter.FromUtf8(NativeResult.Action);
    Result.TimeExpires =
        NativeResult.TimeExpires == 0 ? TOptional<FDateTime>() : FDateTime::FromUnixTimestamp(NativeResult.TimeExpires);
    Result.ReferenceId = Converter.FromAnsi(NativeResult.ReferenceId);
}

}

REDPOINT_EOS_CODE_GUARD_END()