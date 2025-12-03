// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Presence/GetJoinInfo.h"

#include "RedpointEOSAPI/Templates/Impl/SyncGetStringSignedCount.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Presence
{

REDPOINT_EOSSDK_API_CALL_SYNC_GET_STRING_SIGNED_COUNT_IMPL(GetJoinInfo);

void FGetJoinInfo::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.LocalUserId = *Options.LocalUserId;
    NativeOptions.TargetUserId = *Options.TargetUserId;
}

}

REDPOINT_EOS_CODE_GUARD_END()