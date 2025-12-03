// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSNetworking/GameplayDebugger/P2PConnectionGDC.h"

#if WITH_GAMEPLAY_DEBUGGER && !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "Containers/Ticker.h"
#include "Delegates/Delegate.h"
#include "Engine/Canvas.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "RedpointEOSAPI/P2P/GetNATType.h"
#include "RedpointEOSAPI/P2P/GetPacketQueueInfo.h"
#include "RedpointEOSAPI/P2P/GetRelayControl.h"
#include "RedpointEOSAPI/P2P/QueryNATType.h"
#include "RedpointEOSCore/Utils/WorldResolution.h"
#include "RedpointEOSNetworking/Socket/SocketRole.h"
#include "RedpointEOSNetworking/Socket/SocketSubsystemRedpointEOS.h"
#include "SceneView.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if WITH_GAMEPLAY_DEBUGGER && !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(2834297394, Redpoint::EOS::Networking::GameplayDebugger)
{
using namespace ::Redpoint::EOS::API::P2P;
using namespace ::Redpoint::EOS::Core::Utils;
using namespace ::Redpoint::EOS::Networking::Socket;

FP2PConnectionGameplayDebuggerCategory::FP2PConnectionGameplayDebuggerCategory()
{
    this->bShowOnlyWithDebugActor = false;

    SetDataPackReplication<FRepData>(&DataPack);
}

void FP2PConnectionGameplayDebuggerCategory::CollectData(APlayerController *OwnerPC, AActor *DebugActor)
{
}

bool FP2PConnectionGameplayDebuggerCategory::PerformNATQuery(float DeltaSeconds, TWeakObjectPtr<UWorld> World)
{
    if (this->bIsQueryingNAT)
    {
        return false;
    }
    if (!World.Get())
    {
        this->bHasRegisteredPeriodicNATCheck = false;
        return false;
    }

    auto PlatformHandle = FWorldResolution::TryGetPlatformHandle(World.Get());
    if (!PlatformHandle.IsValid())
    {
        this->bHasRegisteredPeriodicNATCheck = false;
        return false;
    }

    this->bIsQueryingNAT = true;
    FQueryNATType::Execute(
        PlatformHandle.ToSharedRef(),
        FQueryNATType::Options{},
        FQueryNATType::CompletionDelegate::CreateSPLambda(
            this,
            [this, World, PlatformHandle](const FQueryNATType::Result &Info) {
                this->bIsQueryingNAT = false;
                if (World.IsValid())
                {
                    if (FGetNATType::Execute(
                            PlatformHandle.ToSharedRef(),
                            FGetNATType::Options{},
                            this->QueriedNATType) != EOS_EResult::EOS_Success)
                    {
                        this->QueriedNATType = EOS_ENATType::EOS_NAT_Unknown;
                    }

                    this->NextNATCheck = FTSTicker::GetCoreTicker().AddTicker(
                        FTickerDelegate::CreateSP(
                            this,
                            &FP2PConnectionGameplayDebuggerCategory::PerformNATQuery,
                            World),
                        60.0f);
                }
                else
                {
                    this->bHasRegisteredPeriodicNATCheck = false;
                }
            }));
    return false;
}

void FP2PConnectionGameplayDebuggerCategory::DrawData(
    APlayerController *OwnerPC,
    FGameplayDebuggerCanvasContext &CanvasContext)
{
    auto Actor = CanvasContext.Canvas->SceneView->ViewActor;
    if (!IsValid(Actor))
    {
        CanvasContext.Printf(TEXT("{red}No view actor\n"));
        return;
    }

    auto *World = Actor->GetWorld();
    if (!IsValid(World))
    {
        CanvasContext.Printf(TEXT("{red}No world\n"));
        return;
    }

    auto PlatformHandle = FWorldResolution::TryGetPlatformHandle(World);
    if (!PlatformHandle.IsValid())
    {
        CanvasContext.Printf(TEXT("{red}No EOS platform handle available\n"));
        return;
    }

    if (!this->bHasRegisteredPeriodicNATCheck)
    {
        this->NextNATCheck = FTSTicker::GetCoreTicker().AddTicker(
            FTickerDelegate::CreateSP(
                this,
                &FP2PConnectionGameplayDebuggerCategory::PerformNATQuery,
                TWeakObjectPtr<UWorld>(World)),
            60.0f);
        this->bHasRegisteredPeriodicNATCheck = true;

        // Immediately kick off a check.
        this->PerformNATQuery(0.0f, World);
    }

    CanvasContext.Printf(TEXT("{cyan}[NAT type in SDK]{white}"));
    if (this->bIsQueryingNAT)
    {
        CanvasContext.Printf(TEXT("NAT type: Querying..."));
    }
    else
    {
        switch (this->QueriedNATType)
        {
        case EOS_ENATType::EOS_NAT_Moderate:
            CanvasContext.Printf(TEXT("NAT type: {orange}Moderate{white}"));
            break;
        case EOS_ENATType::EOS_NAT_Open:
            CanvasContext.Printf(TEXT("NAT type: {green}Open{white}"));
            break;
        case EOS_ENATType::EOS_NAT_Strict:
            CanvasContext.Printf(TEXT("NAT type: {red}Strict{white}"));
            break;
        case EOS_ENATType::EOS_NAT_Unknown:
        default:
            CanvasContext.Printf(TEXT("NAT type: Unknown"));
            break;
        }
    }

    FGetRelayControl::Result RelayControl;
    if (FGetRelayControl::Execute(PlatformHandle.ToSharedRef(), FGetRelayControl::Options{}, RelayControl) ==
        EOS_EResult::EOS_Success)
    {
        CanvasContext.Printf(TEXT("{cyan}[Relay control in SDK]{white}"));
        switch (RelayControl)
        {
        case EOS_ERelayControl::EOS_RC_AllowRelays:
            CanvasContext.Printf(TEXT("Status: Allow relays"));
            break;
        case EOS_ERelayControl::EOS_RC_ForceRelays:
            CanvasContext.Printf(TEXT("Status: {red}Force relays{white}"));
            break;
        case EOS_ERelayControl::EOS_RC_NoRelays:
            CanvasContext.Printf(TEXT("Status: {red}No relays{white}"));
            break;
        default:
            CanvasContext.Printf(TEXT("Status: {red}Unknown{white}"));
            break;
        }
    }

    FGetPacketQueueInfo::Result Info;
    if (FGetPacketQueueInfo::Execute(PlatformHandle.ToSharedRef(), FGetPacketQueueInfo::Options{}, Info) ==
        EOS_EResult::EOS_Success)
    {
        CanvasContext.Printf(TEXT("{cyan}[Packet queue status in SDK]{white}"));
        CanvasContext.Printf(
            TEXT("Incoming packet queue: Packet count: %llu, Size in bytes: %llu/%llu"),
            Info.IncomingPacketQueueCurrentPacketCount,
            Info.IncomingPacketQueueCurrentSizeBytes,
            Info.IncomingPacketQueueMaxSizeBytes);
        CanvasContext.Printf(
            TEXT("Outgoing packet queue: Packet count: %llu, Size in bytes: %llu/%llu"),
            Info.OutgoingPacketQueueCurrentPacketCount,
            Info.OutgoingPacketQueueCurrentSizeBytes,
            Info.OutgoingPacketQueueMaxSizeBytes);
    }

    auto SocketSubsystem = PlatformHandle->GetSystem<FSocketSubsystemRedpointEOS>();
    CanvasContext.Printf(TEXT("{cyan}[Plugin sockets (%d)]{white}"), SocketSubsystem->HeldSockets.Num());
    for (const auto &KV : SocketSubsystem->HeldSockets)
    {
        TArray<FString> Tags;

        Tags.Add(KV.Value->Role->GetRoleName().ToString());

        if (KV.Value->BindAddress.IsValid())
        {
            Tags.Add(FString::Printf(TEXT("BA: {orange}%s{white}"), *KV.Value->BindAddress->ToString(true)));
        }
        if (KV.Value->SocketKey.IsValid())
        {
            Tags.Add(FString::Printf(TEXT("K: {orange}%s{white}"), *KV.Value->SocketKey->ToString(false)));
        }
        {
            FString Threshold = "green";
            if (KV.Value->ReceivedPacketsQueueCount > 5)
            {
                Threshold = "orange";
            }
            if (KV.Value->ReceivedPacketsQueueCount > 20)
            {
                Threshold = "red";
            }
            Tags.Add(FString::Printf(TEXT("PPRQ: {%s}%d{white}"), *Threshold, KV.Value->ReceivedPacketsQueueCount));
        }

        CanvasContext.Printf(TEXT("{white}%d: %s"), KV.Key, *FString::Join(Tags, TEXT(", ")));
    }
}

TSharedRef<FGameplayDebuggerCategory> FP2PConnectionGameplayDebuggerCategory::MakeInstance()
{
    return MakeShareable(new FP2PConnectionGameplayDebuggerCategory());
}

void FP2PConnectionGameplayDebuggerCategory::FRepData::Serialize(FArchive &Ar)
{
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()
