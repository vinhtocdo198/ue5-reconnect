// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/AntiCheatServer/NotifyClientAuthStatusChanged.h"

#if WITH_SERVER_CODE
#include "RedpointEOSAPI/Templates/Impl/Event.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if WITH_SERVER_CODE

namespace Redpoint::EOS::API::AntiCheatServer
{

REDPOINT_EOSSDK_API_CALL_EVENT_IMPL(NotifyClientAuthStatusChanged);

void FNotifyClientAuthStatusChanged::MapOptions(
    NativeOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
}

void FNotifyClientAuthStatusChanged::MapResult(
    Result &Result,
    const NativeResult &NativeResult,
    NativeConverter &Converter)
{
    Result.ClientHandle = NativeResult.ClientHandle;
    Result.ClientAuthStatus = NativeResult.ClientAuthStatus;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()