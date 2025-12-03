// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/RTCAudio/NotifyAudioDevicesChanged.h"

#include "RedpointEOSAPI/Templates/Impl/Event.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::RTCAudio
{

REDPOINT_EOSSDK_API_CALL_EVENT_IMPL(NotifyAudioDevicesChanged);

void FNotifyAudioDevicesChanged::MapOptions(
    NativeOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
}

void FNotifyAudioDevicesChanged::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
}

}

REDPOINT_EOS_CODE_GUARD_END()