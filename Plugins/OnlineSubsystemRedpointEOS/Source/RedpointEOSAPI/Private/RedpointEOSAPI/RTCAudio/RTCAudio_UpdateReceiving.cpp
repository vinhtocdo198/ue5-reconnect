// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/RTCAudio/UpdateReceiving.h"

#include "RedpointEOSAPI/Templates/Impl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::RTCAudio
{

REDPOINT_EOSSDK_API_CALL_ASYNC_IMPL(UpdateReceiving);

void FUpdateReceiving::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.LocalUserId = *Options.LocalUserId;
    NativeOptions.RoomName = Allocator.AsAnsi(Options.RoomName);
    NativeOptions.ParticipantId = *Options.ParticipantId;
    NativeOptions.bAudioEnabled = (*Options.bAudioEnabled) == EAudioEnabled::Yes ? EOS_TRUE : EOS_FALSE;
}

void FUpdateReceiving::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.ResultCode = NativeResult.ResultCode;
    Result.LocalUserId = NativeResult.LocalUserId;
    Result.RoomName = Converter.FromAnsi(NativeResult.RoomName);
    Result.ParticipantId = NativeResult.ParticipantId;
    Result.bAudioEnabled = (NativeResult.bAudioEnabled == EOS_TRUE) ? EAudioEnabled::Yes : EAudioEnabled::No;
}

}

REDPOINT_EOS_CODE_GUARD_END()