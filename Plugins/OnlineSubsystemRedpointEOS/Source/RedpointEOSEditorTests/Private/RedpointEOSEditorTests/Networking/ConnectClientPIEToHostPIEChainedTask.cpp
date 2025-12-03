// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSEditorTests/Networking/ConnectClientPIEToHostPIEChainedTask.h"

#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "IpNetDriver.h"
#include "RedpointEOSCore/Utils/WorldResolution.h"
#include "RedpointEOSEditorTests/Logging.h"
#include "RedpointEOSNetworking/NetDriverResolution.h"
#include "RedpointEOSNetworking/NetworkingSystem.h"
#include "Sockets.h"
#include "UnrealEngine.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3432554009, Redpoint::EOS::Editor::Tests::Networking)
{
using namespace ::Redpoint::EOS::Core::Utils;
using namespace ::Redpoint::EOS::Editor::Tests;
using namespace ::Redpoint::EOS::Networking;

FConnectClientPIEToHostPIEChainedTask::FConnectClientPIEToHostPIEChainedTask(
    int InHostInstanceIndex,
    int InClientInstanceIndex,
    uint16 InPort,
    FSimpleDelegate InBeforeBrowse,
    double InTimeoutSeconds)
    : HostInstanceIndex(InHostInstanceIndex)
    , ClientInstanceIndex(InClientInstanceIndex)
    , Port(InPort)
    , ExtraParams()
    , TimeoutAt()
    , TimeoutSeconds(InTimeoutSeconds)
    , ExistingControllersOnHost()
    , BeforeBrowse(InBeforeBrowse)
{
}

bool FConnectClientPIEToHostPIEChainedTask::ConnectClientPIEToHostPIE(
    float DeltaSeconds,
    FChainedTaskContextRef Context,
    FOnComplete OnDone)
{
    if (FPlatformTime::Seconds() > this->TimeoutAt)
    {
        Context->Assert().TestTrue(TEXT("Did not start client connection to host beacon within timeout"), false);
        OnDone.ExecuteIfBound(false);
        return false;
    }

    FString URL = TEXT("");
    for (const FWorldContext &WorldContext : GEngine->GetWorldContexts())
    {
        if (WorldContext.WorldType == EWorldType::PIE && WorldContext.World() != nullptr &&
            WorldContext.PIEInstance == this->HostInstanceIndex)
        {
            for (const auto &NetDriver : WorldContext.ActiveNetDrivers)
            {
                if (NetDriver.NetDriverDef->DefName.IsEqual(NAME_GameNetDriver))
                {
                    TOptional<FString> ListeningURL;
                    auto PlatformHandle = FNetDriverResolution::TryGetPlatformHandle(NetDriver.NetDriver);
                    if (PlatformHandle.IsValid())
                    {
                        ListeningURL = PlatformHandle->GetSystem<INetworkingSystem>()->GetNetDriverLocalListeningURL(
                            NetDriver.NetDriver,
                            this->Port);
                    }
                    if (ListeningURL.IsSet())
                    {
                        URL = ListeningURL.GetValue();
                    }
                    else if (
                        IsValid(NetDriver.NetDriver) && NetDriver.NetDriver->GetClass() == UIpNetDriver::StaticClass())
                    {
                        UE_LOG(
                            LogRedpointEOSEditorTests,
                            Error,
                            TEXT("Game net driver was IP net driver, not EOS net driver! Make sure the "
                                 "project is set up correctly in order to run this unit test!"));
                        Context->Assert().TestTrue(
                            TEXT("Project misconfigured or EOS net driver could not be loaded"),
                            false);
                        OnDone.ExecuteIfBound(false);
                        return false;
                    }
                    break;
                }
            }
            break;
        }
    }
    if (URL.IsEmpty())
    {
        // Host not ready.
        return true;
    }

    for (const FWorldContext &WorldContext : GEngine->GetWorldContexts())
    {
        if (WorldContext.WorldType == EWorldType::PIE && WorldContext.World() != nullptr &&
            WorldContext.PIEInstance == this->ClientInstanceIndex)
        {
            this->BeforeBrowse.ExecuteIfBound();
            FURL OldURL;
            FURL NewURL(&OldURL, *URL, ETravelType::TRAVEL_Absolute);
            FString Error;
            if (GEngine->Browse((FWorldContext &)WorldContext, NewURL, Error) == EBrowseReturnVal::Failure)
            {
                Context->Assert().TestTrue(FString::Printf(TEXT("Browse failed for client: %s"), *Error), false);
            }
            FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateSP(
                this,
                &FConnectClientPIEToHostPIEChainedTask::WaitForClientPIEToConnectToHostPIE,
                Context,
                OnDone));
            return false;
        }
    }

    return true;
}

bool FConnectClientPIEToHostPIEChainedTask::WaitForClientPIEToConnectToHostPIE(
    float DeltaSeconds,
    FChainedTaskContextRef Context,
    FOnComplete OnDone)
{
    if (FPlatformTime::Seconds() > this->TimeoutAt)
    {
        // We didn't connect in time. Fail the test.
        for (const FWorldContext &WorldContext : GEngine->GetWorldContexts())
        {
            if (WorldContext.WorldType == EWorldType::PIE && WorldContext.World() != nullptr)
            {
                UE_LOG(
                    LogRedpointEOSEditorTests,
                    Verbose,
                    TEXT("Test is failing. PIE world context %d had player controllers:"),
                    WorldContext.PIEInstance);
                for (TActorIterator<APlayerController> It(WorldContext.World()); It; ++It)
                {
                    UE_LOG(LogRedpointEOSEditorTests, Verbose, TEXT("- %s (%d)"), *It->GetName(), It->GetLocalRole());
                }
            }
        }

        UE_LOG(
            LogRedpointEOSEditorTests,
            Verbose,
            TEXT("Expected: Network connection was successfully established within %f seconds"),
            this->TimeoutSeconds);
        Context->Assert().TestTrue(
            FString::Printf(
                TEXT("Network connection was successfully established within %f seconds"),
                this->TimeoutSeconds),
            false);
        OnDone.ExecuteIfBound(false);
        return false;
    }

    // Check to see if the host got a new player controller.
    bool bFoundAuthorativePCOnHost = false;
    bool bFoundAutonomousProxyPCOnClient = false;
    for (const FWorldContext &WorldContext : GEngine->GetWorldContexts())
    {
        if (WorldContext.WorldType == EWorldType::PIE && WorldContext.World() != nullptr)
        {
            for (TActorIterator<APlayerController> It(WorldContext.World()); It; ++It)
            {
                if (WorldContext.PIEInstance == this->HostInstanceIndex &&
                    !this->ExistingControllersOnHost.Contains(*It) && It->GetLocalRole() == ENetRole::ROLE_Authority)
                {
                    bFoundAuthorativePCOnHost = true;
                }

                if (WorldContext.PIEInstance == this->ClientInstanceIndex &&
                    It->GetLocalRole() == ENetRole::ROLE_AutonomousProxy)
                {
                    bFoundAutonomousProxyPCOnClient = true;
                }
            }
        }
    }
    if (bFoundAuthorativePCOnHost && bFoundAutonomousProxyPCOnClient)
    {
        OnDone.ExecuteIfBound(true);
        return false;
    }

    // Still waiting...
    return true;
}

void FConnectClientPIEToHostPIEChainedTask::ExecuteAsync(
    const FChainedTaskContextRef &Context,
    const FOnComplete &OnDone)
{
    this->TimeoutAt = FPlatformTime::Seconds() + this->TimeoutSeconds;

    for (const FWorldContext &WorldContext : GEngine->GetWorldContexts())
    {
        if (WorldContext.WorldType == EWorldType::PIE && WorldContext.World() != nullptr)
        {
            for (TActorIterator<APlayerController> It(WorldContext.World()); It; ++It)
            {
                if (WorldContext.PIEInstance == this->HostInstanceIndex)
                {
                    this->ExistingControllersOnHost.Add(*It);
                }
            }
        }
    }

    FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateSP(
        this,
        &FConnectClientPIEToHostPIEChainedTask::ConnectClientPIEToHostPIE,
        Context,
        OnDone));
}

}

REDPOINT_EOS_CODE_GUARD_END()