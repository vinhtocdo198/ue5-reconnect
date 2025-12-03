// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Lobby/CreateLobbySearch.h"

#include "RedpointEOSAPI/Templates/Impl/SyncReleasable.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Lobby
{

REDPOINT_EOSSDK_API_CALL_SYNC_RELEASABLE_IMPL(CreateLobbySearch);

void FCreateLobbySearch::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.MaxResults = *Options.MaxResults;
}

}

REDPOINT_EOS_CODE_GUARD_END()