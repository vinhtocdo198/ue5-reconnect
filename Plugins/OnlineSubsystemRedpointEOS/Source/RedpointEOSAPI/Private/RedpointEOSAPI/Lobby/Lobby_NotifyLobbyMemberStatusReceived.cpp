// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Lobby/NotifyLobbyMemberStatusReceived.h"

#include "RedpointEOSAPI/Templates/Impl/Event.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Lobby
{

REDPOINT_EOSSDK_API_CALL_EVENT_IMPL(NotifyLobbyMemberStatusReceived);

void FNotifyLobbyMemberStatusReceived::MapOptions(
    NativeOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
}

void FNotifyLobbyMemberStatusReceived::MapResult(
    Result &Result,
    const NativeResult &NativeResult,
    NativeConverter &Converter)
{
    Result.LobbyId = Converter.FromAnsi(NativeResult.LobbyId);
    Result.TargetUserId = NativeResult.TargetUserId;
    Result.CurrentStatus = NativeResult.CurrentStatus;
}

}

REDPOINT_EOS_CODE_GUARD_END()