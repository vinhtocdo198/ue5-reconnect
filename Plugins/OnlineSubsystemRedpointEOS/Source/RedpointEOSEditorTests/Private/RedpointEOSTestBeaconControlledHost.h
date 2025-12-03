// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "OnlineBeaconHost.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "UObject/ObjectMacros.h"

#include "RedpointEOSTestBeaconControlledHost.generated.h"

UCLASS(transient, notplaceable, HideDropdown)
class ARedpointEOSTestBeaconControlledHost : public AOnlineBeaconHost
{
    GENERATED_BODY()

public:
    UPROPERTY()
    TArray<FString> ExtraParams;

    UPROPERTY()
    FString BeaconInstanceName;

    virtual bool InitHost() override;
};