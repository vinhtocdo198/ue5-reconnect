// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/LobbyDetails/CopyMemberInfo.h"

#include "RedpointEOSAPI/Templates/Impl/ObjectSyncCopy.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::LobbyDetails
{

REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_COPY_IMPL(CopyMemberInfo);

void FCopyMemberInfo::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.TargetUserId = *Options.TargetUserId;
}

void FCopyMemberInfo::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.UserId = NativeResult.UserId;
    Result.Platform = (Common::EOnlinePlatformType)NativeResult.Platform;
    Result.bAllowsCrossplay = NativeResult.bAllowsCrossplay == EOS_TRUE ? true : false;
}

}

REDPOINT_EOS_CODE_GUARD_END()