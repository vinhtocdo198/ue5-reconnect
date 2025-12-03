// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Connect/CopyProductUserExternalAccounts.h"

#include "RedpointEOSAPI/Templates/Impl/SyncListCopy.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Connect
{

REDPOINT_EOSSDK_API_CALL_SYNC_LIST_COPY_IMPL(CopyProductUserExternalAccounts);

void FCopyProductUserExternalAccounts::MapCountOptions(
    NativeCountOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
    NativeOptions.TargetUserId = *Options.TargetUserId;
}

void FCopyProductUserExternalAccounts::MapCopyByIndexOptions(
    NativeCopyByIndexOptions &NativeOptions,
    const Options &Options,
    uint32_t Index,
    NativeAllocator &Allocator)
{
    NativeOptions.TargetUserId = *Options.TargetUserId;
    NativeOptions.ExternalAccountInfoIndex = Index;
}

void FCopyProductUserExternalAccounts::MapCopyByIndexResult(
    ResultEntry &Result,
    const NativeCopyByIndexResult &NativeResult,
    NativeConverter &Converter)
{
    Result.ProductUserId = NativeResult.ProductUserId;
    Result.DisplayName = Converter.FromUtf8(NativeResult.DisplayName);
    Result.AccountId = Converter.FromAnsi(NativeResult.AccountId);
    Result.AccountIdType = NativeResult.AccountIdType;
    Result.LastLoginTime = NativeResult.LastLoginTime == EOS_CONNECT_TIME_UNDEFINED
                               ? TOptional<int64>()
                               : TOptional<int64>(NativeResult.LastLoginTime);
}

}

REDPOINT_EOS_CODE_GUARD_END()