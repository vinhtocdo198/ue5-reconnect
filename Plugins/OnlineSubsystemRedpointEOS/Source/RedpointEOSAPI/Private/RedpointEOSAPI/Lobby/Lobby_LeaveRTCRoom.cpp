// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Lobby/LeaveRTCRoom.h"

#if EOS_VERSION_AT_LEAST(1, 17, 0)
#include "RedpointEOSAPI/Templates/Impl/Async.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if EOS_VERSION_AT_LEAST(1, 17, 0)

namespace Redpoint::EOS::API::Lobby
{

REDPOINT_EOSSDK_API_CALL_ASYNC_IMPL(LeaveRTCRoom);

void FLeaveRTCRoom::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.LobbyId = Allocator.AsAnsi(*Options.LobbyId);
    NativeOptions.LocalUserId = *Options.LocalUserId;
}

void FLeaveRTCRoom::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.ResultCode = NativeResult.ResultCode;
    Result.LobbyId = Converter.FromAnsi(NativeResult.LobbyId);
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()