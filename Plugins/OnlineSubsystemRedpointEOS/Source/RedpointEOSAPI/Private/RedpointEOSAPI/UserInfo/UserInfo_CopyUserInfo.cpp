// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/UserInfo/CopyUserInfo.h"

#include "RedpointEOSAPI/Templates/Impl/SyncCopy.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::UserInfo
{

REDPOINT_EOSSDK_API_CALL_SYNC_COPY_IMPL(CopyUserInfo);

void FCopyUserInfo::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.LocalUserId = *Options.LocalUserId;
    NativeOptions.TargetUserId = *Options.TargetUserId;
}

void FCopyUserInfo::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.Country = Converter.FromUtf8(NativeResult.Country);
    Result.DisplayName = Converter.FromUtf8(NativeResult.DisplayName);
    Result.PreferredLanguage = Converter.FromUtf8(NativeResult.PreferredLanguage);
    Result.Nickname = Converter.FromUtf8(NativeResult.Nickname);
    Result.DisplayNameSanitized = Converter.FromUtf8(NativeResult.DisplayNameSanitized);
}

}

REDPOINT_EOS_CODE_GUARD_END()