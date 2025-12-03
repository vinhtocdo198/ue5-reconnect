// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/AntiCheatServer/BeginSession.h"

#if WITH_SERVER_CODE
#include "RedpointEOSAPI/Templates/Impl/Sync.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if WITH_SERVER_CODE

namespace Redpoint::EOS::API::AntiCheatServer
{

REDPOINT_EOSSDK_API_CALL_SYNC_IMPL(BeginSession);

void FBeginSession::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.RegisterTimeoutSeconds = *Options.RegisterTimeoutSeconds;
    NativeOptions.ServerName = Allocator.AsUtf8(*Options.ServerName);
    NativeOptions.bEnableGameplayData = *Options.bEnableGameplayData ? EOS_TRUE : EOS_FALSE;
    NativeOptions.LocalUserId = *Options.LocalUserId;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()