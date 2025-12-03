// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Sessions/NotifySessionInviteRejected.h"

#include "RedpointEOSAPI/Templates/Impl/Event.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Sessions
{

REDPOINT_EOSSDK_API_CALL_EVENT_IMPL(NotifySessionInviteRejected);

void FNotifySessionInviteRejected::MapOptions(
    NativeOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
}

void FNotifySessionInviteRejected::MapResult(
    Result &Result,
    const NativeResult &NativeResult,
    NativeConverter &Converter)
{
    Result.InviteId = Converter.FromAnsi(NativeResult.InviteId);
    Result.LocalUserId = NativeResult.LocalUserId;
    Result.TargetUserId = NativeResult.TargetUserId;
    Result.SessionId = Converter.FromAnsi(NativeResult.SessionId);
}

}

REDPOINT_EOS_CODE_GUARD_END()