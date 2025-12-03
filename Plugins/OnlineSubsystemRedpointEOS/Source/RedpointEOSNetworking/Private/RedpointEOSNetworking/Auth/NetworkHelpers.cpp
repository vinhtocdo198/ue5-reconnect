// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSNetworking/Auth/NetworkHelpers.h"

#include "RedpointEOSNetDriver.h"
#include "RedpointEOSNetworking/NetDriverImpl.h"
#include "RedpointEOSNetworking/Socket/SocketSubsystemRedpointEOS.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2294846777, Redpoint::EOS::Networking::Auth)
{
using namespace ::Redpoint::EOS::Networking;
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
using namespace ::Redpoint::EOS::Networking::Socket;
#endif

const Redpoint::EOS::Config::IConfig *FNetworkHelpers::GetConfig(UNetConnection *Connection)
{
    TSharedPtr<FPlatformInstance> PlatformHandle;
    GetPlatformHandle(Connection, PlatformHandle);
    if (PlatformHandle.IsValid())
    {
        return &PlatformHandle->GetEnvironment()->Config.Get();
    }
    return nullptr;
}

ENetDriverRole FNetworkHelpers::GetRole(UNetConnection *Connection)
{
    auto LocalNetDriver = Cast<URedpointEOSNetDriver>(Connection->Driver);
    if (!IsValid(LocalNetDriver))
    {
        return (ENetDriverRole)-1;
    }
    return LocalNetDriver->GetImpl()->GetEOSRole();
}

void FNetworkHelpers::GetPlatformHandle(UNetConnection *Connection, TSharedPtr<FPlatformInstance> &OutPlatformHandle)
{
    auto Driver = Cast<URedpointEOSNetDriver>(Connection->Driver);
    if (!IsValid(Driver))
    {
        OutPlatformHandle.Reset();
        return;
    }

    auto PlatformHandle = Driver->GetImpl()->GetPlatformHandle();
    checkf(
        PlatformHandle.IsSet(),
        TEXT("The networking driver is not the full EOS implementation, because Epic Online Services failed to "
             "initialize at game startup. Refer to the logs earlier for the actual error."));
    if (PlatformHandle.IsSet())
    {
        OutPlatformHandle = PlatformHandle.GetValue();
    }
    else
    {
        OutPlatformHandle = nullptr;
    }
}

void FNetworkHelpers::GetAntiCheat(
    UNetConnection *Connection,
    TSharedPtr<FPlatformInstance> &OutPlatformHandle,
    TSharedPtr<FAntiCheatSession> &OutAntiCheatSession,
    bool &OutIsBeacon)
{
    auto LocalNetDriver = Cast<URedpointEOSNetDriver>(Connection->Driver);
    if (!IsValid(LocalNetDriver))
    {
        OutIsBeacon = false;
        return;
    }

    auto Impl = LocalNetDriver->GetImpl();

    auto PlatformHandle = Impl->GetPlatformHandle();
    if (PlatformHandle.IsSet())
    {
        OutPlatformHandle = PlatformHandle.GetValue();
    }
    else
    {
        OutPlatformHandle = nullptr;
    }

    OutAntiCheatSession = Impl->AntiCheatSession;
    OutIsBeacon = Impl->bIsOwnedByBeacon;
}

ISocketSubsystem *FNetworkHelpers::GetSocketSubsystem(UNetConnection *Connection)
{
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    TSharedPtr<FPlatformInstance> PlatformHandle;
    GetPlatformHandle(Connection, PlatformHandle);
    if (PlatformHandle.IsValid())
    {
        return &PlatformHandle->GetSystem<FSocketSubsystemRedpointEOS>().Get();
    }
#endif
    return nullptr;
}

}

REDPOINT_EOS_CODE_GUARD_END()