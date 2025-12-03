// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Sessions/NotifyLeaveSessionRequested.h"

#include "RedpointEOSAPI/Templates/Impl/Event.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Sessions
{

REDPOINT_EOSSDK_API_CALL_EVENT_IMPL(NotifyLeaveSessionRequested);

void FNotifyLeaveSessionRequested::MapOptions(
    NativeOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
}

void FNotifyLeaveSessionRequested::MapResult(
    Result &Result,
    const NativeResult &NativeResult,
    NativeConverter &Converter)
{
    Result.LocalUserId = NativeResult.LocalUserId;
    Result.SessionName = Converter.FromAnsi(NativeResult.SessionName);
}

}

REDPOINT_EOS_CODE_GUARD_END()