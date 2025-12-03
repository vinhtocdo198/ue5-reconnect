// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/RTCAudio/NotifyAudioInputState.h"

#include "RedpointEOSAPI/Templates/Impl/Event.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::RTCAudio
{

REDPOINT_EOSSDK_API_CALL_EVENT_IMPL(NotifyAudioInputState);

void FNotifyAudioInputState::MapOptions(
    NativeOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
    NativeOptions.LocalUserId = *Options.LocalUserId;
    NativeOptions.RoomName = Allocator.AsAnsi(*Options.RoomName);
}

void FNotifyAudioInputState::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.LocalUserId = NativeResult.LocalUserId;
    Result.RoomName = Converter.FromAnsi(NativeResult.RoomName);
    Result.Status = NativeResult.Status;
}

}

REDPOINT_EOS_CODE_GUARD_END()