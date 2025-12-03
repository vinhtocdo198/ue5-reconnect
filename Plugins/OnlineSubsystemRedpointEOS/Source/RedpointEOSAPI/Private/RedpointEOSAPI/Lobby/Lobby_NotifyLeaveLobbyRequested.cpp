// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Lobby/NotifyLeaveLobbyRequested.h"

#include "RedpointEOSAPI/Templates/Impl/Event.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Lobby
{

REDPOINT_EOSSDK_API_CALL_EVENT_IMPL(NotifyLeaveLobbyRequested);

void FNotifyLeaveLobbyRequested::MapOptions(
    NativeOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
}

void FNotifyLeaveLobbyRequested::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.LocalUserId = NativeResult.LocalUserId;
    Result.LobbyId = Converter.FromAnsi(NativeResult.LobbyId);
}

}

REDPOINT_EOS_CODE_GUARD_END()