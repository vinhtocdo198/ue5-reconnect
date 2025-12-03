// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Lobby/UpdateLobbyModification.h"

#include "RedpointEOSAPI/Templates/Impl/SyncReleasable.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Lobby
{

REDPOINT_EOSSDK_API_CALL_SYNC_RELEASABLE_IMPL(UpdateLobbyModification);

void FUpdateLobbyModification::MapOptions(
    NativeOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
    NativeOptions.LocalUserId = *Options.LocalUserId;
    NativeOptions.LobbyId = Allocator.AsAnsi(*Options.LobbyId);
}

}

REDPOINT_EOS_CODE_GUARD_END()