// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/SessionSearch/SetSessionId.h"

#include "RedpointEOSAPI/Templates/Impl/ObjectSync.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::SessionSearch
{

REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_IMPL(SetSessionId);

void FSetSessionId::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.SessionId = Allocator.AsAnsi(*Options.SessionId);
}

}

REDPOINT_EOS_CODE_GUARD_END()