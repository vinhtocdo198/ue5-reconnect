// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Lobby/GetRTCRoomName.h"

#include "RedpointEOSAPI/Templates/Impl/SyncGetString.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Lobby
{

REDPOINT_EOSSDK_API_CALL_SYNC_GET_STRING_IMPL(GetRTCRoomName);

void FGetRTCRoomName::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.LocalUserId = *Options.LocalUserId;
    NativeOptions.LobbyId = Allocator.AsAnsi(*Options.LobbyId);
}

}

REDPOINT_EOS_CODE_GUARD_END()