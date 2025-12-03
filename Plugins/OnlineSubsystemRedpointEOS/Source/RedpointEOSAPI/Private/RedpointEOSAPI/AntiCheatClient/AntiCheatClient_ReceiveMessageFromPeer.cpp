// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/AntiCheatClient/ReceiveMessageFromPeer.h"

#if WITH_CLIENT_CODE
#include "RedpointEOSAPI/Templates/Impl/Sync.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if WITH_CLIENT_CODE

namespace Redpoint::EOS::API::AntiCheatClient
{

REDPOINT_EOSSDK_API_CALL_SYNC_IMPL(ReceiveMessageFromPeer);

void FReceiveMessageFromPeer::MapOptions(
    NativeOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
    NativeOptions.Data = *Options.Data;
    NativeOptions.DataLengthBytes = *Options.DataLengthBytes;
    NativeOptions.PeerHandle = *Options.PeerHandle;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()