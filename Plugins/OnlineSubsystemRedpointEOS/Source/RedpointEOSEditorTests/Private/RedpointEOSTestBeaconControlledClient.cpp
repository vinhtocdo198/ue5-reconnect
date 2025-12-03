// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSTestBeaconControlledClient.h"
#include "Net/UnrealNetwork.h"
#include "RedpointEOSEditorTests/Logging.h"

ARedpointEOSTestBeaconControlledClient::ARedpointEOSTestBeaconControlledClient()
{
    this->bReplicates = true;
    this->bIsClientReady = false;
}

void ARedpointEOSTestBeaconControlledClient::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty> &OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ARedpointEOSTestBeaconControlledClient, PIEInstance);
}

void ARedpointEOSTestBeaconControlledClient::OnFailure()
{
    using namespace ::Redpoint::EOS::Editor::Tests;
    // @note: OnFailure is expected to happen when the host forces us to disconnect.
    // UE_LOG(LogRedpointEOSEditorTests, Error, TEXT("Beacon %s encountered an error."), *this->BeaconInstanceName);
    Super::OnFailure();
}

void ARedpointEOSTestBeaconControlledClient::ServerPing_Implementation()
{
    using namespace ::Redpoint::EOS::Editor::Tests;
    UE_LOG(
        LogRedpointEOSEditorTests,
        Verbose,
        TEXT("Server %s received ping, sending pong back."),
        *this->BeaconInstanceName);
    this->ClientPong();
}

void ARedpointEOSTestBeaconControlledClient::ClientPong_Implementation()
{
    using namespace ::Redpoint::EOS::Editor::Tests;
    UE_LOG(
        LogRedpointEOSEditorTests,
        Verbose,
        TEXT("Client %s received pong, marking as received."),
        *this->BeaconInstanceName);
    this->bClientGotPong = true;
    if (this->bClientAutoClose)
    {
        this->DestroyBeacon();
    }
}

void ARedpointEOSTestBeaconControlledClient::ClientMarkAsReady_Implementation()
{
    using namespace ::Redpoint::EOS::Editor::Tests;
    UE_LOG(
        LogRedpointEOSEditorTests,
        Verbose,
        TEXT("Client %s successfully established connection."),
        *this->BeaconInstanceName);
    this->bIsClientReady = true;
}