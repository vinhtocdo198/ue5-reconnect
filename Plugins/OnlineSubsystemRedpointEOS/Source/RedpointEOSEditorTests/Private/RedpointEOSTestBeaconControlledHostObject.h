// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "OnlineBeaconHostObject.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "UObject/ObjectMacros.h"

#include "RedpointEOSTestBeaconControlledHostObject.generated.h"

UCLASS(transient, notplaceable, HideDropdown)
class ARedpointEOSTestBeaconControlledHostObject : public AOnlineBeaconHostObject
{
    GENERATED_BODY()

public:
    ARedpointEOSTestBeaconControlledHostObject();

    UPROPERTY()
    int PIEInstance;

    UPROPERTY()
    FString BeaconInstanceName;

    virtual AOnlineBeaconClient *SpawnBeaconActor(class UNetConnection *ClientConnection) override;
    virtual void OnClientConnected(class AOnlineBeaconClient *NewClientActor, class UNetConnection *ClientConnection)
        override;

    virtual bool Init();

    const TArray<TObjectPtr<AOnlineBeaconClient>> &GetClientActors()
    {
        return this->ClientActors;
    }
};