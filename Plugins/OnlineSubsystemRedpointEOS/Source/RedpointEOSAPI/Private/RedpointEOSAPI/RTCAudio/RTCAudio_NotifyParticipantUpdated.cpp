// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/RTCAudio/NotifyParticipantUpdated.h"

#include "RedpointEOSAPI/Templates/Impl/Event.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::RTCAudio
{

REDPOINT_EOSSDK_API_CALL_EVENT_IMPL(NotifyParticipantUpdated);

void FNotifyParticipantUpdated::MapOptions(
    NativeOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
    NativeOptions.LocalUserId = *Options.LocalUserId;
    NativeOptions.RoomName = Allocator.AsAnsi(*Options.RoomName);
}

void FNotifyParticipantUpdated::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.LocalUserId = NativeResult.LocalUserId;
    Result.RoomName = Converter.FromAnsi(NativeResult.RoomName);
    Result.ParticipantId = NativeResult.ParticipantId;
    Result.bSpeaking = NativeResult.bSpeaking == EOS_TRUE;
    Result.AudioStatus = NativeResult.AudioStatus;
}

}

REDPOINT_EOS_CODE_GUARD_END()