// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/RTC/NotifyDisconnected.h"

#include "RedpointEOSAPI/Templates/Impl/Event.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::RTC
{

REDPOINT_EOSSDK_API_CALL_EVENT_IMPL(NotifyDisconnected);

void FNotifyDisconnected::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.LocalUserId = *Options.LocalUserId;
    NativeOptions.RoomName = Allocator.AsAnsi(*Options.RoomName);
}

void FNotifyDisconnected::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.ResultCode = NativeResult.ResultCode;
    Result.LocalUserId = NativeResult.LocalUserId;
    Result.RoomName = Converter.FromAnsi(NativeResult.RoomName);
}

}

REDPOINT_EOS_CODE_GUARD_END()