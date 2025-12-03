// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Presence/NotifyOnPresenceChanged.h"

#include "RedpointEOSAPI/Templates/Impl/Event.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Presence
{

REDPOINT_EOSSDK_API_CALL_EVENT_IMPL(NotifyOnPresenceChanged);

void FNotifyOnPresenceChanged::MapOptions(
    NativeOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
}

void FNotifyOnPresenceChanged::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.LocalUserId = NativeResult.LocalUserId;
    Result.PresenceUserId = NativeResult.PresenceUserId;
}

}

REDPOINT_EOS_CODE_GUARD_END()