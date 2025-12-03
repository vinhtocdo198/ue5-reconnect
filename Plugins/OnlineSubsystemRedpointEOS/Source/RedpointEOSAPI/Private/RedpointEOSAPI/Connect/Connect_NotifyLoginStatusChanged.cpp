// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Connect/NotifyLoginStatusChanged.h"

#include "RedpointEOSAPI/Templates/Impl/Event.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Connect
{

REDPOINT_EOSSDK_API_CALL_EVENT_IMPL(NotifyLoginStatusChanged);

void FNotifyLoginStatusChanged::MapOptions(
    NativeOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
}

void FNotifyLoginStatusChanged::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.LocalUserId = NativeResult.LocalUserId;
    Result.PreviousStatus = NativeResult.PreviousStatus;
    Result.CurrentStatus = NativeResult.CurrentStatus;
}

}

REDPOINT_EOS_CODE_GUARD_END()