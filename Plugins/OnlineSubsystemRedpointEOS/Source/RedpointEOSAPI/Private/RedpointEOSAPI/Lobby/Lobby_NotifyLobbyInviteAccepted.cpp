// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Lobby/NotifyLobbyInviteAccepted.h"

#include "RedpointEOSAPI/Templates/Impl/Event.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Lobby
{

REDPOINT_EOSSDK_API_CALL_EVENT_IMPL(NotifyLobbyInviteAccepted);

void FNotifyLobbyInviteAccepted::MapOptions(
    NativeOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
}

void FNotifyLobbyInviteAccepted::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.InviteId = Converter.FromAnsi(NativeResult.InviteId);
    Result.LocalUserId = NativeResult.LocalUserId;
    Result.TargetUserId = NativeResult.TargetUserId;
    Result.LobbyId = Converter.FromAnsi(NativeResult.LobbyId);
}

}

REDPOINT_EOS_CODE_GUARD_END()