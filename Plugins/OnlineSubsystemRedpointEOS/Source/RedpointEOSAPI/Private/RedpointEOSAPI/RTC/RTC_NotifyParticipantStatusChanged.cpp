// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/RTC/NotifyParticipantStatusChanged.h"

#include "RedpointEOSAPI/Templates/Impl/Event.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::RTC
{

REDPOINT_EOSSDK_API_CALL_EVENT_IMPL(NotifyParticipantStatusChanged);

void FNotifyParticipantStatusChanged::MapOptions(
    NativeOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
    NativeOptions.LocalUserId = *Options.LocalUserId;
    NativeOptions.RoomName = Allocator.AsAnsi(*Options.RoomName);
}

void FNotifyParticipantStatusChanged::MapResult(
    Result &Result,
    const NativeResult &NativeResult,
    NativeConverter &Converter)
{
    Result.LocalUserId = NativeResult.LocalUserId;
    Result.RoomName = Converter.FromAnsi(NativeResult.RoomName);
    Result.ParticipantId = NativeResult.ParticipantId;
    Result.ParticipantStatus = NativeResult.ParticipantStatus;
    for (uint32_t i = 0; i < NativeResult.ParticipantMetadataCount; i++)
    {
        FString MetadataKey = Converter.FromUtf8(NativeResult.ParticipantMetadata[i].Key);
        FString MetadataValue = Converter.FromUtf8(NativeResult.ParticipantMetadata[i].Value);
        Result.ParticipantMetadata.Add(MetadataKey, MetadataValue);
    }
    Result.bParticipantInBlocklist = NativeResult.bParticipantInBlocklist == EOS_TRUE;
}

}

REDPOINT_EOS_CODE_GUARD_END()