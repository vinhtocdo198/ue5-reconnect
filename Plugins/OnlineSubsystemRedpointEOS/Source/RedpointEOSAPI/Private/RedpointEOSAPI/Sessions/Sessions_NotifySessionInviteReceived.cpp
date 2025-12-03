// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Sessions/NotifySessionInviteReceived.h"

#include "RedpointEOSAPI/Templates/Impl/Event.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Sessions
{

REDPOINT_EOSSDK_API_CALL_EVENT_IMPL(NotifySessionInviteReceived);

void FNotifySessionInviteReceived::MapOptions(
    NativeOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
}

void FNotifySessionInviteReceived::MapResult(
    Result &Result,
    const NativeResult &NativeResult,
    NativeConverter &Converter)
{
    Result.LocalUserId = NativeResult.LocalUserId;
    Result.TargetUserId = NativeResult.TargetUserId;
    Result.InviteId = Converter.FromAnsi(NativeResult.InviteId);
}

}

REDPOINT_EOS_CODE_GUARD_END()