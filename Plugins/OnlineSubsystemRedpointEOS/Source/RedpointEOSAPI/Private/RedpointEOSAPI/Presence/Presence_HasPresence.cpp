// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Presence/HasPresence.h"

#include "RedpointEOSAPI/Templates/Impl/SyncResult.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Presence
{

REDPOINT_EOSSDK_API_CALL_SYNC_RESULT_IMPL(HasPresence);

void FHasPresence::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.LocalUserId = *Options.LocalUserId;
    NativeOptions.TargetUserId = *Options.TargetUserId;
}

FHasPresence::Result FHasPresence::MapResult(const NativeResult &NativeResult)
{
    return NativeResult == EOS_TRUE;
}

}

REDPOINT_EOS_CODE_GUARD_END()