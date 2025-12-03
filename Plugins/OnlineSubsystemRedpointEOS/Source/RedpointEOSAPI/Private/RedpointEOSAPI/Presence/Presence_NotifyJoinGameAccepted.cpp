// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Presence/NotifyJoinGameAccepted.h"

#include "RedpointEOSAPI/Templates/Impl/Event.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Presence
{

REDPOINT_EOSSDK_API_CALL_EVENT_IMPL(NotifyJoinGameAccepted);

void FNotifyJoinGameAccepted::MapOptions(
    NativeOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
}

void FNotifyJoinGameAccepted::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.JoinInfo = NativeResult.JoinInfo;
    Result.LocalUserId = NativeResult.LocalUserId;
    Result.TargetUserId = NativeResult.TargetUserId;
    Result.UiEventId = NativeResult.UiEventId;
}

}

REDPOINT_EOS_CODE_GUARD_END()