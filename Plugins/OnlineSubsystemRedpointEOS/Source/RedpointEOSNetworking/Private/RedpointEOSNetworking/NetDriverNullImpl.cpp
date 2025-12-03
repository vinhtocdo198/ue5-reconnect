// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSNetworking/NetDriverNullImpl.h"

#include "RedpointEOSIpNetConnection.h"
#include "RedpointEOSNetworking/Logging.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2019029752, Redpoint::EOS::Networking)
{
using namespace ::Redpoint::EOS::Networking;

FNetDriverNullImpl::FNetDriverNullImpl(const TSoftObjectPtr<URedpointEOSNetDriver> &InNetDriver)
    : NetDriver(InNetDriver)
{
}

void FNetDriverNullImpl::RequestConnectionClose(URedpointEOSNetConnection *InConnection)
{
    checkf(
        false,
        TEXT("%s: This function should not be called if the networking driver is in null mode."),
        ANSI_TO_TCHAR(__FUNCTION__));
}

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

bool FNetDriverNullImpl::IsIpMode() const
{
    return true;
}

TOptional<UE::Online::FAccountId> FNetDriverNullImpl::GetRegisteredListeningUser() const
{
    return TOptional<UE::Online::FAccountId>();
}

TSharedPtr<FSocketRedpointEOS> FNetDriverNullImpl::GetRedpointEOSSocket() const
{
    return nullptr;
}

#endif

ENetDriverRole FNetDriverNullImpl::GetEOSRole()
{
    // @note: This call should only be used by Anti-Cheat code, which won't run for the null implementation.
    return ENetDriverRole::DedicatedServer;
}

TOptional<FPlatformHandle> FNetDriverNullImpl::GetPlatformHandle()
{
    // @note: Null implementation does not have a platform handle.
    return TOptional<FPlatformHandle>();
}

bool FNetDriverNullImpl::IsNetResourceValid()
{
    if (!this->NetDriver.IsValid())
    {
        return false;
    }

    return this->NetDriver->UIpNetDriver_IsNetResourceValid();
}

ISocketSubsystem *FNetDriverNullImpl::GetSocketSubsystem(URedpointEOSNetDriver *NetDriverPtr)
{
    return NetDriverPtr->UIpNetDriver_GetSocketSubsystem();
}

bool FNetDriverNullImpl::InitConnect(FNetworkNotify *InNotify, const FURL &InConnectURL, FString &Error)
{
    UE_LOG(
        LogRedpointEOSNetworking,
        Error,
        TEXT("Epic Online Services has not been initialized correctly at game startup. Ensure that you have correctly "
             "set the client ID and secret in Project Settings, and if this is a dedicated server, ensure that the "
             "client ID and secret are being passed in on the command line if in dual distribution mode (the "
             "default)."));

    this->NetDriver->NetConnectionClass = URedpointEOSIpNetConnection::StaticClass();
    return this->NetDriver->UIpNetDriver_InitConnect(InNotify, InConnectURL, Error);
}

bool FNetDriverNullImpl::InitListen(
    FNetworkNotify *InNotify,
    FURL &ListenURL,
    bool bReuseAddressAndPort,
    FString &Error)
{
    UE_LOG(
        LogRedpointEOSNetworking,
        Error,
        TEXT("Epic Online Services has not been initialized correctly at game startup. Ensure that you have correctly "
             "set the client ID and secret in Project Settings, and if this is a dedicated server, ensure that the "
             "client ID and secret are being passed in on the command line if in dual distribution mode (the "
             "default)."));

    this->NetDriver->NetConnectionClass = URedpointEOSIpNetConnection::StaticClass();
    return this->NetDriver->UIpNetDriver_InitListen(InNotify, ListenURL, bReuseAddressAndPort, Error);
}

void FNetDriverNullImpl::InitConnectionlessHandler()
{
    this->NetDriver->UIpNetDriver_InitConnectionlessHandler();
}

void FNetDriverNullImpl::TickDispatch(float DeltaTime)
{
    this->NetDriver->UIpNetDriver_TickDispatch(DeltaTime);
}

void FNetDriverNullImpl::TickFlush(float DeltaTime)
{
    this->NetDriver->UIpNetDriver_TickFlush(DeltaTime);
}

void FNetDriverNullImpl::LowLevelSend(
    TSharedPtr<const FInternetAddr> Address,
    void *Data,
    int32 CountBits,
    FOutPacketTraits &Traits)
{
    this->NetDriver->UIpNetDriver_LowLevelSend(Address, Data, CountBits, Traits);
}

void FNetDriverNullImpl::LowLevelDestroy(URedpointEOSNetDriver *NetDriverPtr)
{
    if (IsValid(NetDriverPtr))
    {
        NetDriverPtr->UIpNetDriver_LowLevelDestroy();
    }
}

}

REDPOINT_EOS_CODE_GUARD_END()