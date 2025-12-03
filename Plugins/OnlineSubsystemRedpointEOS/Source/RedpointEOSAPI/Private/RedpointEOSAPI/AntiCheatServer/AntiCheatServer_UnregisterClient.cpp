// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/AntiCheatServer/UnregisterClient.h"

#if WITH_SERVER_CODE
#include "RedpointEOSAPI/Templates/Impl/Sync.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if WITH_SERVER_CODE

namespace Redpoint::EOS::API::AntiCheatServer
{

REDPOINT_EOSSDK_API_CALL_SYNC_IMPL(UnregisterClient);

void FUnregisterClient::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.ClientHandle = *Options.ClientHandle;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()