// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/RTCAudio/UpdateSendingVolume.h"

#include "RedpointEOSAPI/Templates/Impl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::RTCAudio
{

REDPOINT_EOSSDK_API_CALL_ASYNC_IMPL(UpdateSendingVolume);

void FUpdateSendingVolume::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.LocalUserId = *Options.LocalUserId;
    NativeOptions.RoomName = Allocator.AsAnsi(Options.RoomName);
    NativeOptions.Volume = *Options.Volume;
}

void FUpdateSendingVolume::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.LocalUserId = NativeResult.LocalUserId;
    Result.ResultCode = NativeResult.ResultCode;
    Result.Volume = NativeResult.Volume;
}

}

REDPOINT_EOS_CODE_GUARD_END()