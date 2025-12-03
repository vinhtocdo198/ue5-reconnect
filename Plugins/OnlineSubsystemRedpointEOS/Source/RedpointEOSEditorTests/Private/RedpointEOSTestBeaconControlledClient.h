// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "OnlineBeaconClient.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "UObject/ObjectMacros.h"

#include "RedpointEOSTestBeaconControlledClient.generated.h"

UCLASS(transient, notplaceable, HideDropdown)
class ARedpointEOSTestBeaconControlledClient : public AOnlineBeaconClient
{
    GENERATED_BODY()

public:
    ARedpointEOSTestBeaconControlledClient();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;

    UPROPERTY()
    bool bIsClientReady;

    UPROPERTY()
    bool bClientGotPong;

    UPROPERTY()
    bool bClientAutoClose;

    UPROPERTY(Replicated)
    int PIEInstance;

    UPROPERTY()
    FString BeaconInstanceName;

    virtual void OnFailure() override;

    UFUNCTION(server, reliable)
    virtual void ServerPing();

    UFUNCTION(client, reliable)
    virtual void ClientPong();

    UFUNCTION(client, reliable)
    virtual void ClientMarkAsReady();
};