// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Connect/CopyProductUserExternalAccountByAccountType.h"

#include "RedpointEOSAPI/Templates/Impl/SyncCopy.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Connect
{

REDPOINT_EOSSDK_API_CALL_SYNC_COPY_IMPL(CopyProductUserExternalAccountByAccountType);

void FCopyProductUserExternalAccountByAccountType::MapOptions(
    NativeOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
    NativeOptions.TargetUserId = *Options.TargetUserId;
    NativeOptions.AccountIdType = *Options.AccountIdType;
}

void FCopyProductUserExternalAccountByAccountType::MapResult(
    Result &Result,
    const NativeResult &NativeResult,
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