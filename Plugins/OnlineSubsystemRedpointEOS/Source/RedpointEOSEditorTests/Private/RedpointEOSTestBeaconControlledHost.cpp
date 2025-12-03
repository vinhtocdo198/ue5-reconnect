// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSTestBeaconControlledHost.h"

bool ARedpointEOSTestBeaconControlledHost::InitHost()
{
    FURL URL(nullptr, TEXT(""), TRAVEL_Absolute);

    for (const auto &Param : this->ExtraParams)
    {
        URL.AddOption(*Param);
    }

    URL.Port = this->ListenPort;
    if (URL.Valid)
    {
        if (this->InitBase() && this->NetDriver)
        {
            FString Error;
            if (this->NetDriver->InitListen(this, URL, false, Error))
            {
                this->ListenPort = URL.Port;
                NetDriver->SetWorld(this->GetWorld());
                NetDriver->Notify = this;
                NetDriver->InitialConnectTimeout = this->BeaconConnectionInitialTimeout;
                NetDriver->ConnectionTimeout = this->BeaconConnectionTimeout;
                return true;
            }
            else
            {
                // error initializing the network stack...
                UE_LOG(LogBeacon, Log, TEXT("AOnlineBeaconHost::InitHost failed"));
                this->OnFailure();
            }
        }
    }

    return false;
}