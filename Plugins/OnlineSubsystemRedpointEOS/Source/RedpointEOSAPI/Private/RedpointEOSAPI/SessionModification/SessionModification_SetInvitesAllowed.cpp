// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/SessionModification/SetInvitesAllowed.h"

#include "RedpointEOSAPI/Templates/Impl/ObjectSync.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::SessionModification
{

REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_IMPL(SetInvitesAllowed);

void FSetInvitesAllowed::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.bInvitesAllowed = *Options.bInvitesAllowed;
}

}

REDPOINT_EOS_CODE_GUARD_END()