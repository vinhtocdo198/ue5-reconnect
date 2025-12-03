// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/SessionModification/SetPermissionLevel.h"

#include "RedpointEOSAPI/Templates/Impl/ObjectSync.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::SessionModification
{

REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_IMPL(SetPermissionLevel);

void FSetPermissionLevel::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.PermissionLevel = *Options.PermissionLevel;
}

}

REDPOINT_EOS_CODE_GUARD_END()