// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/AntiCheatClient/RegisterPeer.h"

#if WITH_CLIENT_CODE
#include "RedpointEOSAPI/Templates/Impl/Sync.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if WITH_CLIENT_CODE

namespace Redpoint::EOS::API::AntiCheatClient
{

REDPOINT_EOSSDK_API_CALL_SYNC_IMPL(RegisterPeer);

void FRegisterPeer::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.PeerHandle = *Options.PeerHandle;
    NativeOptions.ClientType = *Options.ClientType;
    NativeOptions.ClientPlatform = *Options.ClientPlatform;
    NativeOptions.AuthenticationTimeout = *Options.AuthenticationTimeout;
    NativeOptions.IpAddress = Allocator.AsAnsi(*Options.IpAddress);
    NativeOptions.PeerProductUserId = *Options.PeerProductUserId;
    NativeOptions.AccountId_DEPRECATED = nullptr;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()