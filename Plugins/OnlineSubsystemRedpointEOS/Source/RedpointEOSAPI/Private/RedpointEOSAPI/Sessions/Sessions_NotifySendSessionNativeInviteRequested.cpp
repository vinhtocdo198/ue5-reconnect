// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Sessions/NotifySendSessionNativeInviteRequested.h"

#include "RedpointEOSAPI/Templates/Impl/Event.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Sessions
{

REDPOINT_EOSSDK_API_CALL_EVENT_IMPL(NotifySendSessionNativeInviteRequested);

void FNotifySendSessionNativeInviteRequested::MapOptions(
    NativeOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
}

void FNotifySendSessionNativeInviteRequested::MapResult(
    Result &Result,
    const NativeResult &NativeResult,
    NativeConverter &Converter)
{
    Result.UiEventId = NativeResult.UiEventId;
    Result.LocalUserId = NativeResult.LocalUserId;
    Result.TargetNativeAccountType = NativeResult.TargetNativeAccountType;
    Result.TargetUserNativeAccountId = Converter.FromAnsi(NativeResult.TargetUserNativeAccountId);
    Result.SessionId = Converter.FromAnsi(NativeResult.SessionId);
}

}

REDPOINT_EOS_CODE_GUARD_END()