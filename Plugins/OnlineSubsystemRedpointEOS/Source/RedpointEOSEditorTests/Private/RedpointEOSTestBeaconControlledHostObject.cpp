// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSTestBeaconControlledHostObject.h"

#include "RedpointEOSEditorTests/Logging.h"
#include "RedpointEOSTestBeaconControlledClient.h"

ARedpointEOSTestBeaconControlledHostObject::ARedpointEOSTestBeaconControlledHostObject()
{
    ClientBeaconActorClass = ARedpointEOSTestBeaconControlledClient::StaticClass();
    BeaconTypeName = ClientBeaconActorClass->GetName();
}

bool ARedpointEOSTestBeaconControlledHostObject::Init()
{
    using namespace ::Redpoint::EOS::Editor::Tests;
    UE_LOG(LogRedpointEOSEditorTests, Verbose, TEXT("Beacon is now listening on server."));
    return true;
}

void ARedpointEOSTestBeaconControlledHostObject::OnClientConnected(
    AOnlineBeaconClient *NewClientActor,
    UNetConnection *ClientConnection)
{
    Super::OnClientConnected(NewClientActor, ClientConnection);

    ARedpointEOSTestBeaconControlledClient *BeaconClient = Cast<ARedpointEOSTestBeaconControlledClient>(NewClientActor);
    if (BeaconClient != NULL)
    {
        BeaconClient->ClientMarkAsReady();
    }
}

AOnlineBeaconClient *ARedpointEOSTestBeaconControlledHostObject::SpawnBeaconActor(UNetConnection *ClientConnection)
{
    ARedpointEOSTestBeaconControlledClient *BeaconClient =
        Cast<ARedpointEOSTestBeaconControlledClient>(Super::SpawnBeaconActor(ClientConnection));
    BeaconClient->PIEInstance = this->PIEInstance;
    BeaconClient->BeaconInstanceName = this->BeaconInstanceName;
    return BeaconClient;
}