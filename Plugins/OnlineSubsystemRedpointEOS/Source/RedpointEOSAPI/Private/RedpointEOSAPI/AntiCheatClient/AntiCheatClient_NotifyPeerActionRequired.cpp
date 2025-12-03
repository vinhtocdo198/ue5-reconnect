// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/AntiCheatClient/NotifyPeerActionRequired.h"

#if WITH_CLIENT_CODE
#include "RedpointEOSAPI/Templates/Impl/Event.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if WITH_CLIENT_CODE

namespace Redpoint::EOS::API::AntiCheatClient
{

REDPOINT_EOSSDK_API_CALL_EVENT_IMPL(NotifyPeerActionRequired);

void FNotifyPeerActionRequired::MapOptions(
    NativeOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
}

void FNotifyPeerActionRequired::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.ClientHandle = NativeResult.ClientHandle;
    Result.ClientAction = NativeResult.ClientAction;
    Result.ActionReasonCode = NativeResult.ActionReasonCode;
    Result.ActionReasonDetailsString = Converter.FromUtf8(NativeResult.ActionReasonDetailsString);
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()