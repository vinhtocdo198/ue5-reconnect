// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/P2P/SendPacket.h"

#if WITH_CLIENT_CODE
#include "RedpointEOSAPI/Templates/Impl/Sync.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if WITH_CLIENT_CODE

namespace Redpoint::EOS::API::P2P
{

REDPOINT_EOSSDK_API_CALL_SYNC_IMPL(SendPacket);

void FSendPacket::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.LocalUserId = *Options.LocalUserId;
    NativeOptions.RemoteUserId = *Options.RemoteUserId;
    NativeOptions.SocketId = AllocateSocketId(Allocator, *Options.SocketId);
    NativeOptions.Channel = *Options.Channel;
    NativeOptions.DataLengthBytes = *Options.DataLengthBytes;
    NativeOptions.Data = *Options.Data;
    NativeOptions.bAllowDelayedDelivery = *Options.bAllowDelayedDelivery ? EOS_TRUE : EOS_FALSE;
    NativeOptions.Reliability = *Options.Reliability;
    NativeOptions.bDisableAutoAcceptConnection = *Options.bDisableAutoAcceptConnection ? EOS_TRUE : EOS_FALSE;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()