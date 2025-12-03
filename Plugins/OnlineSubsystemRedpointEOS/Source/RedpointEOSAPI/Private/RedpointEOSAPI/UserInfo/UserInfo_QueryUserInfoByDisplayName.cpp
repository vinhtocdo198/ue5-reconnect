// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/UserInfo/QueryUserInfoByDisplayName.h"

#include "RedpointEOSAPI/Templates/Impl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::UserInfo
{

REDPOINT_EOSSDK_API_CALL_ASYNC_IMPL(QueryUserInfoByDisplayName);

void FQueryUserInfoByDisplayName::MapOptions(
    NativeOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
    NativeOptions.LocalUserId = *Options.LocalUserId;
    NativeOptions.DisplayName = Allocator.AsUtf8(*Options.DisplayName);
}

void FQueryUserInfoByDisplayName::MapResult(
    Result &Result,
    const NativeResult &NativeResult,
    NativeConverter &Converter)
{
    Result.ResultCode = NativeResult.ResultCode;
    Result.LocalUserId = NativeResult.LocalUserId;
    Result.TargetUserId = NativeResult.TargetUserId;
}

}

REDPOINT_EOS_CODE_GUARD_END()