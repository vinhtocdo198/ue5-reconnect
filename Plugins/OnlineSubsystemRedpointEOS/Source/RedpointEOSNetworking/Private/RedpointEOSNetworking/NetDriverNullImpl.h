// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSNetDriver.h"
#include "RedpointEOSNetworking/NetDriver.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2019029752, Redpoint::EOS::Networking)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Networking;
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
using namespace ::Redpoint::EOS::Networking::Socket;
#endif

class FNetDriverNullImpl : public INetDriver
{
public:
    const TSoftObjectPtr<URedpointEOSNetDriver> NetDriver;

public:
    FNetDriverNullImpl(const TSoftObjectPtr<URedpointEOSNetDriver> &InNetDriver);
    UE_NONCOPYABLE(FNetDriverNullImpl);
    virtual ~FNetDriverNullImpl() override = default;

    virtual void RequestConnectionClose(URedpointEOSNetConnection *InConnection) override;

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    virtual bool IsIpMode() const override;
    virtual TOptional<UE::Online::FAccountId> GetRegisteredListeningUser() const override;
    virtual TSharedPtr<FSocketRedpointEOS> GetRedpointEOSSocket() const override;
#endif

    virtual ENetDriverRole GetEOSRole() override;
    virtual TOptional<FPlatformHandle> GetPlatformHandle() override;

    virtual bool IsNetResourceValid() override;
    virtual ISocketSubsystem *GetSocketSubsystem(URedpointEOSNetDriver *NetDriverPtr) override;
    virtual bool InitConnect(FNetworkNotify *InNotify, const FURL &ConnectURL, FString &Error) override;
    virtual bool InitListen(FNetworkNotify *InNotify, FURL &ListenURL, bool bReuseAddressAndPort, FString &Error)
        override;
    virtual void InitConnectionlessHandler() override;
    virtual void TickDispatch(float DeltaTime) override;
    virtual void TickFlush(float DeltaTime) override;
    virtual void LowLevelSend(
        TSharedPtr<const FInternetAddr> Address,
        void *Data,
        int32 CountBits,
        FOutPacketTraits &Traits) override;
    virtual void LowLevelDestroy(URedpointEOSNetDriver *NetDriverPtr) override;
};

}

namespace Redpoint::EOS::Networking
{
REDPOINT_EOS_FILE_NS_EXPORT(2019029752, Redpoint::EOS::Networking, FNetDriverNullImpl)
}

REDPOINT_EOS_CODE_GUARD_END()