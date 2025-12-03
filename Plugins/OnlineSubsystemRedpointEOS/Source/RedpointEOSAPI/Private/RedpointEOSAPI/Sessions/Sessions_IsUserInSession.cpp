// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Sessions/IsUserInSession.h"

#include "RedpointEOSAPI/Templates/Impl/Sync.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Sessions
{

REDPOINT_EOSSDK_API_CALL_SYNC_IMPL(IsUserInSession);

void FIsUserInSession::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.SessionName = Allocator.AsAnsi(*Options.SessionName);
    NativeOptions.TargetUserId = *Options.TargetUserId;
}

}

REDPOINT_EOS_CODE_GUARD_END()