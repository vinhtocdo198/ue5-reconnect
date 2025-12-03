// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Lobby/UpdateLobby.h"

#include "RedpointEOSAPI/Templates/Impl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Lobby
{

REDPOINT_EOSSDK_API_CALL_ASYNC_IMPL(UpdateLobby);

void FUpdateLobby::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.LobbyModificationHandle = (*Options.LobbyModificationHandle)->Value();
}

void FUpdateLobby::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.ResultCode = NativeResult.ResultCode;
    Result.LobbyId = Converter.FromAnsi(NativeResult.LobbyId);
}

}

REDPOINT_EOS_CODE_GUARD_END()