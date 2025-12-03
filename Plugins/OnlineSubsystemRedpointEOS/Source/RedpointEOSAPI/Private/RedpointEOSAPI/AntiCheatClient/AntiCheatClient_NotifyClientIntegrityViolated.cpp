// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/AntiCheatClient/NotifyClientIntegrityViolated.h"

#if WITH_CLIENT_CODE
#include "RedpointEOSAPI/Templates/Impl/Event.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if WITH_CLIENT_CODE

namespace Redpoint::EOS::API::AntiCheatClient
{

REDPOINT_EOSSDK_API_CALL_EVENT_IMPL(NotifyClientIntegrityViolated);

void FNotifyClientIntegrityViolated::MapOptions(
    NativeOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
}

void FNotifyClientIntegrityViolated::MapResult(
    Result &Result,
    const NativeResult &NativeResult,
    NativeConverter &Converter)
{
    Result.ViolationType = NativeResult.ViolationType;
    Result.ViolationMessage = Converter.FromUtf8(NativeResult.ViolationMessage);
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()