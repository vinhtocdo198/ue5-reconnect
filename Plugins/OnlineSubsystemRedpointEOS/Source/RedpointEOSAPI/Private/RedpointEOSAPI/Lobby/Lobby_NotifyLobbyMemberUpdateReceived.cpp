// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Lobby/NotifyLobbyMemberUpdateReceived.h"

#include "RedpointEOSAPI/Templates/Impl/Event.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Lobby
{

REDPOINT_EOSSDK_API_CALL_EVENT_IMPL(NotifyLobbyMemberUpdateReceived);

void FNotifyLobbyMemberUpdateReceived::MapOptions(
    NativeOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
}

void FNotifyLobbyMemberUpdateReceived::MapResult(
    Result &Result,
    const NativeResult &NativeResult,
    NativeConverter &Converter)
{
    Result.LobbyId = Converter.FromAnsi(NativeResult.LobbyId);
    Result.TargetUserId = NativeResult.TargetUserId;
}

}

REDPOINT_EOS_CODE_GUARD_END()