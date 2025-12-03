// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSIpNetConnection.h"

#include "PacketHandler.h"
#include "RedpointEOSConfig/Config.h"
#include "RedpointEOSCore/Utils/WorldResolution.h"
#include "RedpointEOSNetDriver.h"
#include "RedpointEOSNetworking/Logging.h"
#include "RedpointEOSNetworking/NetDriverImpl.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

void URedpointEOSIpNetConnection::InitBase(
    UNetDriver *InDriver,
    FSocket *InSocket,
    const FURL &InURL,
    EConnectionState InState,
    int32 InMaxPacket,
    int32 InPacketOverhead)
{
    using namespace ::Redpoint::EOS::Core::Utils;
    using namespace ::Redpoint::EOS::Networking;

    UIpConnection::InitBase(InDriver, InSocket, InURL, InState, InMaxPacket, InPacketOverhead);

    auto RedpointEOSDriver = Cast<URedpointEOSNetDriver>(InDriver);
    if (!IsValid(RedpointEOSDriver))
    {
        return;
    }

    auto PlatformHandle = RedpointEOSDriver->GetImpl()->GetPlatformHandle();

    if (PlatformHandle.IsSet())
    {
        auto Config = PlatformHandle.GetValue()->GetEnvironment()->Config;
        if (Config->GetNetworkAuthenticationMode() != ENetworkAuthenticationMode::Off)
        {
            if (!this->Handler->GetEncryptionComponent().IsValid())
            {
                UE_LOG(
                    LogRedpointEOSNetworking,
                    Error,
                    TEXT("You must set [PacketHandlerComponents] EncryptionComponent=AESGCMHandlerComponent in "
                         "DefaultEngine.ini in order to connect to dedicated servers on EOS."));
                this->Close();
            }
        }
    }
}

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()