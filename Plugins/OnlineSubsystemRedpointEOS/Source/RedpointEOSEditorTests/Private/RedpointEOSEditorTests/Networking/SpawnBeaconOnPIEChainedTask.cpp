// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSEditorTests/Networking/SpawnBeaconOnPIEChainedTask.h"

#include "UnrealEngine.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1588596934, Redpoint::EOS::Editor::Tests::Networking)
{

FSpawnBeaconOnPIEChainedTask::FSpawnBeaconOnPIEChainedTask(
    TSharedRef<TSoftObjectPtr<ARedpointEOSTestBeaconControlledHost>> InBeaconHost,
    TSharedRef<TSoftObjectPtr<ARedpointEOSTestBeaconControlledHostObject>> InBeaconHostObject,
    int InInstanceIndex,
    uint16 InPort,
    const FString &InExtraParams)
    : BeaconHost(InBeaconHost)
    , BeaconHostObject(InBeaconHostObject)
    , InstanceIndex(InInstanceIndex)
    , Port(InPort)
    , ExtraParams(InExtraParams)
{
}

void FSpawnBeaconOnPIEChainedTask::ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone)
{
    for (const FWorldContext &WorldContext : GEngine->GetWorldContexts())
    {
        if (WorldContext.WorldType == EWorldType::PIE && WorldContext.World() != nullptr &&
            WorldContext.PIEInstance == this->InstanceIndex)
        {
            ARedpointEOSTestBeaconControlledHost *NewBeaconHost =
                WorldContext.World()->SpawnActor<ARedpointEOSTestBeaconControlledHost>(
                    ARedpointEOSTestBeaconControlledHost::StaticClass());
            NewBeaconHost->ListenPort = this->Port;
            NewBeaconHost->ExtraParams.Add(this->ExtraParams);
            if (Context->Assert().TestTrue(
                    TEXT("Beacon initializes"),
                    NewBeaconHost != nullptr && NewBeaconHost->InitHost()))
            {
                ARedpointEOSTestBeaconControlledHostObject *NewBeaconHostObject =
                    WorldContext.World()->SpawnActor<ARedpointEOSTestBeaconControlledHostObject>(
                        ARedpointEOSTestBeaconControlledHostObject::StaticClass());
                if (Context->Assert().TestTrue(TEXT("Beacon host object created"), NewBeaconHostObject != nullptr))
                {
                    NewBeaconHostObject->PIEInstance = this->InstanceIndex;
                    NewBeaconHost->RegisterHost(NewBeaconHostObject);
                }
                NewBeaconHost->PauseBeaconRequests(false);
                *this->BeaconHost = NewBeaconHost;
                *this->BeaconHostObject = NewBeaconHostObject;
            }
            OnDone.ExecuteIfBound(true);
            return;
        }
    }

    Context->Assert().TestTrue(FString::Printf(TEXT("Expected PIE instance %d to exist"), this->InstanceIndex), false);
    OnDone.ExecuteIfBound(false);
    return;
}

}

REDPOINT_EOS_CODE_GUARD_END()