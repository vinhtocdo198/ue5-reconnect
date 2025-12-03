// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Friends/NotifyBlockedUsersUpdate.h"

#include "RedpointEOSAPI/Templates/Impl/Event.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Friends
{

REDPOINT_EOSSDK_API_CALL_EVENT_IMPL(NotifyBlockedUsersUpdate);

void FNotifyBlockedUsersUpdate::MapOptions(
    NativeOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
}

void FNotifyBlockedUsersUpdate::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.LocalUserId = NativeResult.LocalUserId;
    Result.TargetUserId = NativeResult.TargetUserId;
    Result.bBlocked = NativeResult.bBlocked == EOS_TRUE;
}

}

REDPOINT_EOS_CODE_GUARD_END()