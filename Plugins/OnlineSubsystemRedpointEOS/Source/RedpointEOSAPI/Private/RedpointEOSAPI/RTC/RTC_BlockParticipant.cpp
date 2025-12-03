// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/RTC/BlockParticipant.h"

#include "RedpointEOSAPI/Templates/Impl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::RTC
{

REDPOINT_EOSSDK_API_CALL_ASYNC_IMPL(BlockParticipant);

void FBlockParticipant::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.LocalUserId = *Options.LocalUserId;
    NativeOptions.RoomName = Allocator.AsAnsi(*Options.RoomName);
    NativeOptions.ParticipantId = *Options.ParticipantId;
    NativeOptions.bBlocked = (*Options.bBlocked) ? EOS_TRUE : EOS_FALSE;
}

void FBlockParticipant::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.ResultCode = NativeResult.ResultCode;
    Result.LocalUserId = NativeResult.LocalUserId;
    Result.RoomName = Converter.FromAnsi(NativeResult.RoomName);
    Result.ParticipantId = NativeResult.ParticipantId;
    Result.bBlocked = NativeResult.bBlocked == EOS_TRUE;
}

}

REDPOINT_EOS_CODE_GUARD_END()