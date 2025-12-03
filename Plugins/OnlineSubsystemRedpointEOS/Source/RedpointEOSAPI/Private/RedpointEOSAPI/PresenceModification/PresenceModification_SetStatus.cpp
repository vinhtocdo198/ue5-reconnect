// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/PresenceModification/SetStatus.h"

#include "RedpointEOSAPI/Templates/Impl/ObjectSync.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::PresenceModification
{

REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_IMPL(SetStatus);

void FSetStatus::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.Status = *Options.Status;
}

}

REDPOINT_EOS_CODE_GUARD_END()