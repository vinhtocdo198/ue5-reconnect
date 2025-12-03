// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Lobby/NotifyLobbyUpdateReceived.h"

#include "RedpointEOSAPI/Templates/Impl/Event.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Lobby
{

REDPOINT_EOSSDK_API_CALL_EVENT_IMPL(NotifyLobbyUpdateReceived);

void FNotifyLobbyUpdateReceived::MapOptions(
    NativeOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
}

void FNotifyLobbyUpdateReceived::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.LobbyId = Converter.FromAnsi(NativeResult.LobbyId);
}

}

REDPOINT_EOS_CODE_GUARD_END()