// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Lobby/NotifyLobbyInviteReceived.h"

#include "RedpointEOSAPI/Templates/Impl/Event.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Lobby
{

REDPOINT_EOSSDK_API_CALL_EVENT_IMPL(NotifyLobbyInviteReceived);

void FNotifyLobbyInviteReceived::MapOptions(
    NativeOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
}

void FNotifyLobbyInviteReceived::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.InviteId = Converter.FromAnsi(NativeResult.InviteId);
    Result.LocalUserId = NativeResult.LocalUserId;
    Result.TargetUserId = NativeResult.TargetUserId;
}

}

REDPOINT_EOS_CODE_GUARD_END()