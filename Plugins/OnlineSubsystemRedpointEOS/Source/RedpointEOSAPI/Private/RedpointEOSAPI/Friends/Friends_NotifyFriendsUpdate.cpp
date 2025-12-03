// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Friends/NotifyFriendsUpdate.h"

#include "RedpointEOSAPI/Templates/Impl/Event.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Friends
{

REDPOINT_EOSSDK_API_CALL_EVENT_IMPL(NotifyFriendsUpdate);

void FNotifyFriendsUpdate::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
}

void FNotifyFriendsUpdate::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.LocalUserId = NativeResult.LocalUserId;
    Result.TargetUserId = NativeResult.TargetUserId;
    Result.PreviousStatus = NativeResult.PreviousStatus;
    Result.CurrentStatus = NativeResult.CurrentStatus;
}

}

REDPOINT_EOS_CODE_GUARD_END()