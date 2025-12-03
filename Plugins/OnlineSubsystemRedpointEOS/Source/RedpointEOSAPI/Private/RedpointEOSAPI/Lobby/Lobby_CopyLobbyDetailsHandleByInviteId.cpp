// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Lobby/CopyLobbyDetailsHandleByInviteId.h"

#include "RedpointEOSAPI/Templates/Impl/SyncReleasable.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Lobby
{

REDPOINT_EOSSDK_API_CALL_SYNC_RELEASABLE_IMPL(CopyLobbyDetailsHandleByInviteId);

void FCopyLobbyDetailsHandleByInviteId::MapOptions(
    NativeOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
    NativeOptions.InviteId = Allocator.AsAnsi(*Options.InviteId);
}

}

REDPOINT_EOS_CODE_GUARD_END()