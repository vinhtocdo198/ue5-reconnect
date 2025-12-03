// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/AntiCheatServer/RegisterClient.h"

#if WITH_SERVER_CODE
#include "RedpointEOSAPI/Templates/Impl/Sync.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if WITH_SERVER_CODE

namespace Redpoint::EOS::API::AntiCheatServer
{

REDPOINT_EOSSDK_API_CALL_SYNC_IMPL(RegisterClient);

void FRegisterClient::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.ClientHandle = *Options.ClientHandle;
    NativeOptions.ClientType = *Options.ClientType;
    NativeOptions.ClientPlatform = *Options.ClientPlatform;
    NativeOptions.AccountId_DEPRECATED = nullptr;
    NativeOptions.IpAddress = Allocator.AsAnsi(*Options.IpAddress);
    NativeOptions.UserId = *Options.UserId;
#if EOS_VERSION_AT_LEAST(1, 17, 0)
    NativeOptions.Reserved01 = 0;
#endif
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()