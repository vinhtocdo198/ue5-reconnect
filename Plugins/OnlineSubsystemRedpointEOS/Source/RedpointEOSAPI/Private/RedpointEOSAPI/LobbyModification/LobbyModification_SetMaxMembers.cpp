// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/LobbyModification/SetMaxMembers.h"

#include "RedpointEOSAPI/Templates/Impl/ObjectSync.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::LobbyModification
{

REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_IMPL(SetMaxMembers);

void FSetMaxMembers::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.MaxMembers = *Options.MaxMembers;
}

}

REDPOINT_EOS_CODE_GUARD_END()