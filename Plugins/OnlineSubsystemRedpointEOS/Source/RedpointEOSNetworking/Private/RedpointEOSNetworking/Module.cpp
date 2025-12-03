// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSNetworking/Module.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/CoreDelegates.h"
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSCore/InstancePool.h"
#include "RedpointEOSCore/Utils/WorldResolution.h"
#include "RedpointEOSNetworking/Logging.h"
#include "RedpointEOSNetworking/NetDriverImpl.h"
#include "RedpointEOSNetworking/NetDriverNullImpl.h"
#include "RedpointEOSNetworking/Stats.h"
#include "UObject/Object.h"
#if WITH_GAMEPLAY_DEBUGGER && !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "GameplayDebugger.h"
#include "RedpointEOSNetworking/GameplayDebugger/P2PConnectionGDC.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(731365916, Redpoint::EOS::Networking)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Core;
using namespace ::Redpoint::EOS::Core::Utils;
using namespace ::Redpoint::EOS::Networking;
#if WITH_GAMEPLAY_DEBUGGER && !REDPOINT_EOS_IS_DEDICATED_SERVER
using namespace ::Redpoint::EOS::Networking::GameplayDebugger;
#endif

void FRedpointEOSNetworkingModule::OnNetDriverCreated(UWorld *World, UNetDriver *NetDriver)
{
    checkf(
        IsValid(World),
        TEXT("Currently expect World to always be valid - if not, there is another OnNetDriverCreated scenario "
             "that we need to handle properly!"));

    auto *RedpointNetDriver = Cast<URedpointEOSNetDriver>(NetDriver);
    if (IsValid(RedpointNetDriver) && !RedpointNetDriver->Impl.IsValid())
    {
        UE_LOG(
            LogRedpointEOSNetworking,
            Verbose,
            TEXT("Assigning network driver implementation to new URedpointEOSNetDriver instance."));

        auto PlatformHandle = FWorldResolution::TryGetPlatformHandle(World);
        FPlatformOptionalRefCountedHandle PlatformOptionalRefCountedHandle;
        if (!PlatformHandle.IsValid())
        {
            // In the editor, the networking driver may have been created prior to the platform handle, as
            // Online::GetSubsystem hasn't been called yet. To handle this case, request that the platform handle be
            // created.
            FName InstanceName = FName(TEXT("DefaultInstance"));
            if (GEngine != nullptr)
            {
                auto *WorldContext = GEngine->GetWorldContextFromWorld(World);
                if (WorldContext != nullptr)
                {
                    if (WorldContext->WorldType == EWorldType::PIE)
                    {
                        InstanceName = WorldContext->ContextHandle;
                    }
                }
            }
            PlatformOptionalRefCountedHandle = IInstancePool::Pool().Create(InstanceName);
            if (PlatformOptionalRefCountedHandle.IsValid())
            {
                PlatformHandle = PlatformOptionalRefCountedHandle->Instance();
            }
            else
            {
                UE_LOG(
                    LogRedpointEOSNetworking,
                    Error,
                    TEXT(
                        "Epic Online Services has not been initialized correctly at game startup. Ensure that you have "
                        "correctly set the client ID and secret in Project Settings, and if this is a dedicated "
                        "server, ensure that the client ID and secret are being passed in on the command line if in "
                        "dual distribution mode (the default)."));
            }
        }

        if (PlatformHandle.IsValid())
        {
            RedpointNetDriver->Impl = MakeShared<FNetDriverImpl>(
                World,
                RedpointNetDriver,
                PlatformHandle.ToSharedRef(),
                PlatformOptionalRefCountedHandle);
        }
        else
        {
            RedpointNetDriver->Impl = MakeShared<FNetDriverNullImpl>(RedpointNetDriver);
        }
    }
    else
    {
        UE_LOG(
            LogRedpointEOSNetworking,
            Warning,
            TEXT("Network driver passed to OnNetDriverCreated is not URedpointEOSNetDriver or already has an "
                 "implementation assigned."));
    }
}

void FRedpointEOSNetworkingModule::StartupModule()
{
    this->OnNetDriverCreatedHandle =
        FWorldDelegates::OnNetDriverCreated.AddRaw(this, &FRedpointEOSNetworkingModule::OnNetDriverCreated);

#if WITH_GAMEPLAY_DEBUGGER && !REDPOINT_EOS_IS_DEDICATED_SERVER
    FCoreDelegates::OnPostEngineInit.AddLambda([]() {
        IGameplayDebugger &GameplayDebuggerModule = IGameplayDebugger::Get();
        GameplayDebuggerModule.RegisterCategory(
            "P2PConnections",
            IGameplayDebugger::FOnGetCategory::CreateStatic(&FP2PConnectionGameplayDebuggerCategory::MakeInstance),
            EGameplayDebuggerCategoryState::EnabledInGame,
            5);
        GameplayDebuggerModule.NotifyCategoriesChanged();

#if REDPOINT_EOS_UE_5_5_OR_LATER
        // Verify that our networking drivers are configured correctly, and that our dynamic config layer system works.
        if (IsValid(GEngine))
        {
            for (const auto &NetDriverDefinition : GEngine->NetDriverDefinitions)
            {
                if (NetDriverDefinition.DefName.IsEqual(NAME_GameNetDriver) ||
                    NetDriverDefinition.DefName.IsEqual(NAME_BeaconNetDriver))
                {
                    if (!NetDriverDefinition.DriverClassName.IsEqual(
                            FName(TEXT("/Script/RedpointEOSNetworking.RedpointEOSNetDriver"))))
                    {
                        checkf(
                            false,
                            TEXT("The networking driver for %s is configured to be %s and not "
                                 "/Script/RedpointEOSNetworking.RedpointEOSNetDriver. The dynamic config layer system "
                                 "should be overriding this. Please report this bug."),
                            *NetDriverDefinition.DefName.ToString(),
                            *NetDriverDefinition.DriverClassName.ToString());
                    }
                }
            }
        }
#endif
    });
#endif

    this->OnBeforeAnyTickHandle = IInstancePool::Pool().OnBeforeAnyTick().AddLambda([]() {
        REDPOINT_EOS_INT_COUNTER_SET(P2P, ReceivedLoopIters, 0);
        REDPOINT_EOS_INT_COUNTER_SET(P2P, ReceivedPackets, 0);
        REDPOINT_EOS_INT_COUNTER_SET(P2P, ReceivedBytes, 0);
        REDPOINT_EOS_INT_COUNTER_SET(P2P, SentPackets, 0);
        REDPOINT_EOS_INT_COUNTER_SET(P2P, SentBytes, 0);
    });
};

void FRedpointEOSNetworkingModule::ShutdownModule()
{
    IInstancePool::Pool().OnBeforeAnyTick().Remove(this->OnBeforeAnyTickHandle);
    FWorldDelegates::OnNetDriverCreated.Remove(this->OnNetDriverCreatedHandle);
}

}

REDPOINT_EOS_CODE_GUARD_END()

IMPLEMENT_MODULE(Redpoint::EOS::Networking::FRedpointEOSNetworkingModule, RedpointEOSNetworking);