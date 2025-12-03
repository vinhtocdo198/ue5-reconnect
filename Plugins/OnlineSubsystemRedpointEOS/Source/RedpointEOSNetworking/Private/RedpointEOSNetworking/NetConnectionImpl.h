// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSNetConnection.h"
#include "RedpointEOSNetDriver.h"
#include "RedpointEOSNetworking/ForwardDecls.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "RedpointEOSNetworking/Socket/SocketRedpointEOS.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

class URedpointEOSControlChannel;

namespace REDPOINT_EOS_FILE_NS_ID(2904893824, Redpoint::EOS::Networking)
{
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
using namespace ::Redpoint::EOS::Networking::Socket;
#endif

class FNetConnectionImpl : public TSharedFromThis<FNetConnectionImpl>
{
    friend ::Redpoint::EOS::Networking::FNetDriverImpl;
    friend URedpointEOSControlChannel;

private:
    TSoftObjectPtr<URedpointEOSNetDriver> NetDriver;
    TSoftObjectPtr<URedpointEOSNetConnection> NetConnection;

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    TWeakPtr<FSocketRedpointEOS> Socket;
#endif

    void ReceivePacketFromDriver(const TSharedPtr<FInternetAddr> &ReceivedAddr, uint8 *Buffer, int32 BufferSize);

public:
    FNetConnectionImpl(
        TSoftObjectPtr<URedpointEOSNetDriver> InNetDriver,
        TSoftObjectPtr<URedpointEOSNetConnection> InNetConnection);
    UE_NONCOPYABLE(FNetConnectionImpl);
    ~FNetConnectionImpl() = default;

#if REDPOINT_EOS_ENABLE_SOCKET_LEVEL_NETWORK_TRACING
    static void LogPacket(
        UNetDriver *NetDriver,
        const TSharedPtr<const FInternetAddr> &InRemoteAddr,
        bool bIncoming,
        const uint8 *Data,
        int32 CountBytes);
#endif

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    void SetSocket(const TSharedRef<FSocketRedpointEOS> &InSocket);
#endif

    void LowLevelSend(void *Data, int32 CountBits, FOutPacketTraits &Traits);
    FString LowLevelGetRemoteAddress(bool bAppendPort = false);
    FString LowLevelDescribe();
};

}

namespace Redpoint::EOS::Networking
{
REDPOINT_EOS_FILE_NS_EXPORT(2904893824, Redpoint::EOS::Networking, FNetConnectionImpl)
}

REDPOINT_EOS_CODE_GUARD_END()