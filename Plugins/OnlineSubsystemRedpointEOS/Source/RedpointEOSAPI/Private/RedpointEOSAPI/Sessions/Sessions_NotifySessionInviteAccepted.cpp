// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Sessions/NotifySessionInviteAccepted.h"

#include "RedpointEOSAPI/Templates/Impl/Event.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Sessions
{

REDPOINT_EOSSDK_API_CALL_EVENT_IMPL(NotifySessionInviteAccepted);

void FNotifySessionInviteAccepted::MapOptions(
    NativeOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
}

void FNotifySessionInviteAccepted::MapResult(
    Result &Result,
    const NativeResult &NativeResult,
    NativeConverter &Converter)
{
    Result.SessionId = Converter.FromAnsi(NativeResult.SessionId);
    Result.LocalUserId = NativeResult.LocalUserId;
    Result.TargetUserId = NativeResult.TargetUserId;
    Result.InviteId = Converter.FromAnsi(NativeResult.InviteId);
}

}

REDPOINT_EOS_CODE_GUARD_END()