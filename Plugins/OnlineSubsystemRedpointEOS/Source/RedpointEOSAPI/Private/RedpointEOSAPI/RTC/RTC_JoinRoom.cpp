// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/RTC/JoinRoom.h"

#include "RedpointEOSAPI/Templates/Impl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::RTC
{

REDPOINT_EOSSDK_API_CALL_ASYNC_IMPL(JoinRoom);

void FJoinRoom::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.LocalUserId = *Options.LocalUserId;
    NativeOptions.RoomName = Allocator.AsAnsi(*Options.RoomName);
    NativeOptions.ClientBaseUrl = Allocator.AsUtf8(*Options.ClientBaseUrl);
    NativeOptions.ParticipantToken = Allocator.AsUtf8(*Options.ParticipantToken);
    NativeOptions.ParticipantId = *Options.ParticipantId;
    NativeOptions.Flags = *Options.Flags;
    NativeOptions.bManualAudioInputEnabled = (*Options.bManualAudioInputEnabled) ? EOS_TRUE : EOS_FALSE;
    NativeOptions.bManualAudioOutputEnabled = (*Options.bManualAudioOutputEnabled) ? EOS_TRUE : EOS_FALSE;
}

void FJoinRoom::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.ResultCode = NativeResult.ResultCode;
    Result.LocalUserId = NativeResult.LocalUserId;
    Result.RoomName = Converter.FromAnsi(NativeResult.RoomName);
    for (uint32_t i = 0; i < NativeResult.RoomOptionsCount; i++)
    {
        FString OptionKey = Converter.FromUtf8(NativeResult.RoomOptions[i].Key);
        FString OptionValue = Converter.FromUtf8(NativeResult.RoomOptions[i].Value);
        Result.RoomOptions.Add(OptionKey, OptionValue);
    }
}

}

REDPOINT_EOS_CODE_GUARD_END()