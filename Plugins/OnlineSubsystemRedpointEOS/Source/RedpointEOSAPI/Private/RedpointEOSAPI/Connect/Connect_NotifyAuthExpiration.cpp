// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Connect/NotifyAuthExpiration.h"

#include "RedpointEOSAPI/Templates/Impl/Event.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Connect
{

REDPOINT_EOSSDK_API_CALL_EVENT_IMPL(NotifyAuthExpiration);

void FNotifyAuthExpiration::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
}

void FNotifyAuthExpiration::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.LocalUserId = NativeResult.LocalUserId;
}

}

REDPOINT_EOS_CODE_GUARD_END()