// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Lobby/NotifySendLobbyNativeInviteRequested.h"

#include "RedpointEOSAPI/Templates/Impl/Event.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Lobby
{

REDPOINT_EOSSDK_API_CALL_EVENT_IMPL(NotifySendLobbyNativeInviteRequested);

void FNotifySendLobbyNativeInviteRequested::MapOptions(
    NativeOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
}

void FNotifySendLobbyNativeInviteRequested::MapResult(
    Result &Result,
    const NativeResult &NativeResult,
    NativeConverter &Converter)
{
    Result.UiEventId = NativeResult.UiEventId;
    Result.LocalUserId = NativeResult.LocalUserId;
    Result.TargetNativeAccountType = Converter.FromAnsi(NativeResult.TargetNativeAccountType);
    Result.TargetUserNativeAccountId = Converter.FromAnsi(NativeResult.TargetUserNativeAccountId);
    Result.LobbyId = Converter.FromAnsi(NativeResult.LobbyId);
}

}

REDPOINT_EOS_CODE_GUARD_END()