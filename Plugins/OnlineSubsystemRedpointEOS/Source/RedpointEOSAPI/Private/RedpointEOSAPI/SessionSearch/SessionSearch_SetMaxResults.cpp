// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/SessionSearch/SetMaxResults.h"

#include "RedpointEOSAPI/Templates/Impl/ObjectSync.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::SessionSearch
{

REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_IMPL(SetMaxResults);

void FSetMaxResults::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.MaxSearchResults = *Options.MaxSearchResults;
}

}

REDPOINT_EOS_CODE_GUARD_END()