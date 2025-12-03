// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Friends/GetStatus.h"

#include "RedpointEOSAPI/Templates/Impl/SyncResult.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Friends
{

REDPOINT_EOSSDK_API_CALL_SYNC_RESULT_IMPL(GetStatus);

void FGetStatus::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.LocalUserId = *Options.LocalUserId;
    NativeOptions.TargetUserId = *Options.TargetUserId;
}

FGetStatus::Result FGetStatus::MapResult(const NativeResult &NativeResult)
{
    return NativeResult;
}

}

REDPOINT_EOS_CODE_GUARD_END()