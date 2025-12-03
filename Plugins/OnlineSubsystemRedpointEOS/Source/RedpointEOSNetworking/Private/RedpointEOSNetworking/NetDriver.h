// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Online/CoreOnline.h"
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSAntiCheat/AntiCheatSession.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSNetDriver.h"
#include "RedpointEOSNetworking/NetDriverRole.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(4230421139, Redpoint::EOS::Networking::Socket, FSocketRedpointEOS)
#endif
class URedpointEOSNetConnection;

namespace REDPOINT_EOS_FILE_NS_ID(2449947577, Redpoint::EOS::Networking)
{
using namespace ::Redpoint::EOS::AntiCheat;
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Networking;
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
using namespace ::Redpoint::EOS::Networking::Socket;
#endif

class INetDriver
{
public:
    TSharedPtr<FAntiCheatSession> AntiCheatSession;
    bool bIsOwnedByBeacon;

public:
    INetDriver() = default;
    UE_NONCOPYABLE(INetDriver);
    virtual ~INetDriver() = default;

    virtual void RequestConnectionClose(URedpointEOSNetConnection *InConnection) = 0;

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    virtual bool IsIpMode() const = 0;
    virtual TOptional<UE::Online::FAccountId> GetRegisteredListeningUser() const = 0;
    virtual TSharedPtr<FSocketRedpointEOS> GetRedpointEOSSocket() const = 0;
#endif

    virtual ENetDriverRole GetEOSRole() = 0;
    virtual TOptional<FPlatformHandle> GetPlatformHandle() = 0;

    virtual bool IsNetResourceValid() = 0;
    virtual ISocketSubsystem *GetSocketSubsystem(URedpointEOSNetDriver *NetDriverPtr) = 0;
    virtual bool InitConnect(FNetworkNotify *InNotify, const FURL &ConnectURL, FString &Error) = 0;
    virtual bool InitListen(FNetworkNotify *InNotify, FURL &ListenURL, bool bReuseAddressAndPort, FString &Error) = 0;
    virtual void InitConnectionlessHandler() = 0;
    virtual void TickDispatch(float DeltaTime) = 0;
    virtual void TickFlush(float DeltaTime) = 0;
    virtual void LowLevelSend(
        TSharedPtr<const FInternetAddr> Address,
        void *Data,
        int32 CountBits,
        FOutPacketTraits &Traits) = 0;
    virtual void LowLevelDestroy(URedpointEOSNetDriver *NetDriverPtr) = 0;
};
}

namespace Redpoint::EOS::Networking
{
REDPOINT_EOS_FILE_NS_EXPORT(2449947577, Redpoint::EOS::Networking, INetDriver)
}

REDPOINT_EOS_CODE_GUARD_END()