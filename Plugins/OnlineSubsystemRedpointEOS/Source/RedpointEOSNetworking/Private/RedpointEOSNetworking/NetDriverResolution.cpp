// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSNetworking/NetDriverResolution.h"

#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "OnlineBeacon.h"
#include "RedpointEOSCore/InstancePool.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3872218600, Redpoint::EOS::Networking)
{
using namespace ::Redpoint::EOS::Core;

const FWorldContext *FNetDriverResolution::TryGetWorldContext(const UNetDriver *NetDriver)
{
    // If we don't have a global engine, we can't retrieve world contexts.
    if (GEngine == nullptr)
    {
        return nullptr;
    }

    // If we have an explicit world reference set, then return that.
    UWorld *ExplicitWorld = NetDriver->GetWorld();
    if (IsValid(ExplicitWorld))
    {
        return GEngine->GetWorldContextFromWorld(ExplicitWorld);
    }

    // When a pending net game is calling InitConnect, we're attached to
    // a pending net game but we don't have a world reference. But we can
    // ask the engine for our world context...
    FWorldContext *WorldContextFromPending = GEngine->GetWorldContextFromPendingNetGameNetDriver(NetDriver);
    if (WorldContextFromPending != nullptr)
    {
        return WorldContextFromPending;
    }

    // When the net driver is being created for a beacon, the net driver instance
    // does not get a world set against it until after InitListen/InitConnect happens.
    // However, we need the world before then.
    //
    // The net driver also won't appear as a pending net game, because it's being set
    // up for a custom beacon.
    //
    // In this case, we have to iterate through the worlds, and then iterate through
    // all of the AOnlineBeacon* actors (both host and client beacons) in each world,
    // and see if any of them have this network driver associated with them. If they do,
    // then the world that the beacon is in is the world that we are associated with.
    for (const auto &WorldContext : GEngine->GetWorldContexts())
    {
        UWorld *ItWorld = WorldContext.World();
        if (ItWorld != nullptr)
        {
            for (TActorIterator<AOnlineBeacon> It(ItWorld); It; ++It)
            {
                if (It->GetNetDriver() == NetDriver)
                {
                    return &WorldContext;
                }
            }
        }
    }

    // No world context could be found.
    return nullptr;
}

TSharedPtr<FPlatformInstance> FNetDriverResolution::TryGetPlatformHandle(const UNetDriver *InNetDriver)
{
    const auto *WorldContext = FNetDriverResolution::TryGetWorldContext(InNetDriver);
    if (WorldContext == nullptr)
    {
        return nullptr;
    }
    auto InstanceName =
        WorldContext->WorldType == EWorldType::PIE ? WorldContext->ContextHandle : FName(TEXT("DefaultInstance"));
    auto PlatformHandle = IInstancePool::Pool().TryGetExisting(InstanceName);
    return PlatformHandle;
}

FPlatformHandle FNetDriverResolution::GetPlatformHandle(const UNetDriver *InNetDriver)
{
    auto PlatformHandle = FNetDriverResolution::TryGetPlatformHandle(InNetDriver);
    checkf(
        PlatformHandle.IsValid(),
        TEXT("Unable to locate platform handle when calling FNetDriverResolution::GetPlatformHandle."));
    return PlatformHandle.ToSharedRef();
}

}

REDPOINT_EOS_CODE_GUARD_END()