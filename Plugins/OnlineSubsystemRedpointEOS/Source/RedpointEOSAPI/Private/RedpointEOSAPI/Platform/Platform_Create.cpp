// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Platform/Create.h"

#include "RedpointEOSAPI/Logging.h"
#include "RedpointEOSAPI/Stats.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Platform
{

FCreate::Result FCreate::Execute(const Options &InOptions)
{
    checkf(IsInGameThread(), TEXT("Calls to the EOS SDK can only be made on the game thread!"));
    
    DECLARE_CYCLE_STAT(StatCallName(), STAT_Call, STATGROUP_RedpointEOS);
    SCOPE_CYCLE_COUNTER(STAT_Call);

    UE_LOG(LogRedpointEOSAPI, VeryVerbose, TEXT("[starting] %s"), LogCallName());

    TSharedRef<NativeAllocator> Allocator = MakeShared<NativeAllocator>();

    NativeOptions _NativeOptions = {};
    _NativeOptions.ApiVersion = NativeOptionsVersion();
    MapOptions(_NativeOptions, InOptions, *Allocator);

    return EOS_Platform_Create(&_NativeOptions);
}

void FCreate::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.Reserved = *Options.ReservedOptions;
    NativeOptions.ProductId = Allocator.AsUtf8(Options.ProductId);
    NativeOptions.SandboxId = Allocator.AsUtf8(Options.SandboxId);
    NativeOptions.ClientCredentials.ClientId = Allocator.AsUtf8(Options.ClientId);
    NativeOptions.ClientCredentials.ClientSecret = Allocator.AsUtf8(Options.ClientSecret);
    NativeOptions.bIsServer = *Options.bIsServer;
    NativeOptions.EncryptionKey =
        Allocator.AsUtf8(Options.EncryptionKey, Templates::Util::EAllocationFlags::ReturnNullptrIfEmpty);
    NativeOptions.OverrideCountryCode =
        Allocator.AsUtf8(Options.OverrideCountryCode, Templates::Util::EAllocationFlags::ReturnNullptrIfEmpty);
    NativeOptions.OverrideLocaleCode =
        Allocator.AsUtf8(Options.OverrideLocaleCode, Templates::Util::EAllocationFlags::ReturnNullptrIfEmpty);
    NativeOptions.DeploymentId = Allocator.AsUtf8(Options.DeploymentId);
    NativeOptions.Flags = *Options.Flags;
    NativeOptions.CacheDirectory =
        Allocator.AsUtf8(Options.CacheDirectory, Templates::Util::EAllocationFlags::ReturnNullptrIfEmpty);
    NativeOptions.TickBudgetInMilliseconds = *Options.TickBudgetInMilliseconds;
    NativeOptions.RTCOptions = *Options.RTCOptions;
    NativeOptions.IntegratedPlatformOptionsContainerHandle = nullptr;
    NativeOptions.SystemSpecificOptions = nullptr;
#if EOS_VERSION_AT_LEAST(1, 16, 2)
    NativeOptions.TaskNetworkTimeoutSeconds = nullptr;
#endif
}

}

REDPOINT_EOS_CODE_GUARD_END()