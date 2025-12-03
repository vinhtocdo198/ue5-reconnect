// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/P2P/SetPortRange.h"

#if WITH_CLIENT_CODE
#include "RedpointEOSAPI/Templates/Impl/Sync.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if WITH_CLIENT_CODE

namespace Redpoint::EOS::API::P2P
{

REDPOINT_EOSSDK_API_CALL_SYNC_IMPL(SetPortRange);

void FSetPortRange::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.Port = *Options.Port;
    NativeOptions.MaxAdditionalPortsToTry = *Options.MaxAdditionalPortsToTry;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()