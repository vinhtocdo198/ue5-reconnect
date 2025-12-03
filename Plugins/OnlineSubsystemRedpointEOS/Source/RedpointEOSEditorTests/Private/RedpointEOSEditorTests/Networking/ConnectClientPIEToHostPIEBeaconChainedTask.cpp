// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSEditorTests/Networking/ConnectClientPIEToHostPIEBeaconChainedTask.h"

#include "IpNetDriver.h"
#include "RedpointEOSCore/Utils/WorldResolution.h"
#include "RedpointEOSEditorTests/Logging.h"
#include "RedpointEOSNetworking/NetDriverResolution.h"
#include "RedpointEOSNetworking/NetworkingSystem.h"
#include "Sockets.h"
#include "UnrealEngine.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1401072254, Redpoint::EOS::Editor::Tests::Networking)
{
using namespace ::Redpoint::EOS::Core::Utils;
using namespace ::Redpoint::EOS::Editor::Tests;
using namespace ::Redpoint::EOS::Networking;

FConnectClientPIEToHostPIEBeaconChainedTask::FConnectClientPIEToHostPIEBeaconChainedTask(
    TSharedRef<TSoftObjectPtr<ARedpointEOSTestBeaconControlledClient>> InBeaconClient,
    int InHostInstanceIndex,
    int InClientInstanceIndex,
    uint16 InPort)
    : BeaconClient(InBeaconClient)
    , HostInstanceIndex(InHostInstanceIndex)
    , ClientInstanceIndex(InClientInstanceIndex)
    , Port(InPort)
    , TimeoutAt()
{
}

void FConnectClientPIEToHostPIEBeaconChainedTask::ExecuteAsync(
    const FChainedTaskContextRef &Context,
    const FOnComplete &OnDone)
{
    this->TimeoutAt = FPlatformTime::Seconds() + 10;

    FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateSPLambda(this, [this, Context, OnDone](float) -> bool {
        if (FPlatformTime::Seconds() > this->TimeoutAt)
        {
            Context->Assert().TestTrue(
                TEXT("Did not create client beacon connected to host beacon within timeout"),
                false);
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
                    if (NetDriver.NetDriverDef->DefName.IsEqual(NAME_BeaconNetDriver))
                    {
                        TOptional<FString> ListeningURL;
                        auto PlatformHandle = FNetDriverResolution::TryGetPlatformHandle(NetDriver.NetDriver);
                        if (PlatformHandle.IsValid())
                        {
                            ListeningURL =
                                PlatformHandle->GetSystem<INetworkingSystem>()->GetNetDriverLocalListeningURL(
                                    NetDriver.NetDriver,
                                    this->Port);
                        }
                        if (ListeningURL.IsSet())
                        {
                            URL = ListeningURL.GetValue();
                        }
                        else if (
                            IsValid(NetDriver.NetDriver) &&
                            NetDriver.NetDriver->GetClass() == UIpNetDriver::StaticClass())
                        {
                            UE_LOG(
                                LogRedpointEOSEditorTests,
                                Error,
                                TEXT("Beacon net driver was IP net driver, not EOS net driver! Make sure the "
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

        if (this->BeaconClient->IsNull())
        {
            for (const FWorldContext &WorldContext : GEngine->GetWorldContexts())
            {
                if (WorldContext.WorldType == EWorldType::PIE && WorldContext.World() != nullptr &&
                    WorldContext.PIEInstance == this->ClientInstanceIndex)
                {
                    auto Beacon = WorldContext.World()->SpawnActor<AOnlineBeacon>(
                        ARedpointEOSTestBeaconControlledClient::StaticClass());
                    if (Beacon)
                    {
                        FURL OldURL;
                        FURL NewURL(&OldURL, *URL, ETravelType::TRAVEL_Absolute);
                        Cast<ARedpointEOSTestBeaconControlledClient>(Beacon)->InitClient(NewURL);
                        *this->BeaconClient = Cast<ARedpointEOSTestBeaconControlledClient>(Beacon);
                    }
                    return true;
                }
            }
        }

        if (this->BeaconClient->IsValid() &&
            (*this->BeaconClient)->GetConnectionState() == EBeaconConnectionState::Open)
        {
            OnDone.ExecuteIfBound(true);
            return false;
        }

        return true;
    }));
}

}

REDPOINT_EOS_CODE_GUARD_END()