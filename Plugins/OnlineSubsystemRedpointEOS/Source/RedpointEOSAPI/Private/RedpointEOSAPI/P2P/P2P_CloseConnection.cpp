// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/P2P/CloseConnection.h"

#if WITH_CLIENT_CODE
#include "RedpointEOSAPI/Templates/Impl/Sync.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if WITH_CLIENT_CODE

namespace Redpoint::EOS::API::P2P
{

REDPOINT_EOSSDK_API_CALL_SYNC_IMPL(CloseConnection);

void FCloseConnection::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.LocalUserId = *Options.LocalUserId;
    NativeOptions.RemoteUserId = *Options.RemoteUserId;
    NativeOptions.SocketId = AllocateSocketId(Allocator, *Options.SocketId);
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()