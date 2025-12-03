// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/AntiCheatClient/NotifyPeerAuthStatusChanged.h"

#if WITH_CLIENT_CODE
#include "RedpointEOSAPI/Templates/Impl/Event.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if WITH_CLIENT_CODE

namespace Redpoint::EOS::API::AntiCheatClient
{

REDPOINT_EOSSDK_API_CALL_EVENT_IMPL(NotifyPeerAuthStatusChanged);

void FNotifyPeerAuthStatusChanged::MapOptions(
    NativeOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
}

void FNotifyPeerAuthStatusChanged::MapResult(
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