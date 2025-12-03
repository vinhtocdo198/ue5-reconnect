// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/LobbySearch/SetLobbyId.h"

#include "RedpointEOSAPI/Templates/Impl/ObjectSync.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::LobbySearch
{

REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_IMPL(SetLobbyId);

void FSetLobbyId::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.LobbyId = Allocator.AsAnsi(*Options.LobbyId);
}

}

REDPOINT_EOS_CODE_GUARD_END()