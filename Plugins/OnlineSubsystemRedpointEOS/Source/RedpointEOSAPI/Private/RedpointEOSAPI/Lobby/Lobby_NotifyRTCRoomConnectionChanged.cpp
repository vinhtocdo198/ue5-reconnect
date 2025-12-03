// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Lobby/NotifyRTCRoomConnectionChanged.h"

#include "RedpointEOSAPI/Templates/Impl/Event.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Lobby
{

REDPOINT_EOSSDK_API_CALL_EVENT_IMPL(NotifyRTCRoomConnectionChanged);

void FNotifyRTCRoomConnectionChanged::MapOptions(
    NativeOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
}

void FNotifyRTCRoomConnectionChanged::MapResult(
    Result &Result,
    const NativeResult &NativeResult,
    NativeConverter &Converter)
{
    Result.LobbyId = Converter.FromAnsi(NativeResult.LobbyId);
    Result.LocalUserId = NativeResult.LocalUserId;
    Result.bIsConnected = NativeResult.bIsConnected == EOS_TRUE;
    Result.DisconnectReason = NativeResult.DisconnectReason;
}

}

REDPOINT_EOS_CODE_GUARD_END()