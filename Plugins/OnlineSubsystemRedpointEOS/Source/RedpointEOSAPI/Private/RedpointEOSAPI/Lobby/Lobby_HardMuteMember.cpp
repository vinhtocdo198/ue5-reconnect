// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Lobby/HardMuteMember.h"

#include "RedpointEOSAPI/Templates/Impl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Lobby
{

REDPOINT_EOSSDK_API_CALL_ASYNC_IMPL(HardMuteMember);

void FHardMuteMember::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.LocalUserId = *Options.LocalUserId;
    NativeOptions.LobbyId = Allocator.AsAnsi(*Options.LobbyId);
    NativeOptions.TargetUserId = *Options.TargetUserId;
    NativeOptions.bHardMute = (*Options.bHardMute) ? EOS_TRUE : EOS_FALSE;
}

void FHardMuteMember::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.ResultCode = NativeResult.ResultCode;
    Result.LobbyId = Converter.FromAnsi(NativeResult.LobbyId);
    Result.TargetUserId = NativeResult.TargetUserId;
}

}

REDPOINT_EOS_CODE_GUARD_END()