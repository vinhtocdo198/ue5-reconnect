// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "RedpointEOSAPI/P2P/NotifyPeerConnectionClosed.h"
#include "RedpointEOSNetworking/Socket/InternetAddrRedpointEOS.h"
#include "RedpointEOSNetworking/Socket/SocketRole.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(4230421139, Redpoint::EOS::Networking::Socket, FSocketRedpointEOS)

namespace REDPOINT_EOS_FILE_NS_ID(3448274137, Redpoint::EOS::Networking::Socket)
{
using namespace ::Redpoint::EOS::API::P2P;
using namespace ::Redpoint::EOS::Networking::Socket;

/**
 * Client sockets are outbound connections that are actively made. That is, this machine actively connects to a remote
 * socket (that is in the Listening role), and this socket only ever communicates with that remote socket. Calling
 * SendTo for an address that doesn't match the remote address will fail. Calling RecvFrom will only ever receive
 * packets from that address.
 */
class FSocketRoleClient : public ISocketRole
{
private:
    void OnRemoteConnectionClosed(
        const TSharedRef<FSocketRedpointEOS> &Socket,
        const FNotifyPeerConnectionClosed::Result &Info) const;

public:
    FSocketRoleClient() = default;
    UE_NONCOPYABLE(FSocketRoleClient);
    virtual ~FSocketRoleClient() override = default;

    virtual FName GetRoleName() const override;

    virtual bool Close(FSocketRedpointEOS &Socket) const override;
    virtual bool Connect(FSocketRedpointEOS &Socket, const FInternetAddrRedpointEOS &InDestAddr) const override;
    virtual bool Listen(FSocketRedpointEOS &Socket, int32 MaxBacklog) const override;
    virtual bool Accept(
        FSocketRedpointEOS &Socket,
        TSharedRef<FSocketRedpointEOS> InRemoteSocket,
        const FInternetAddrRedpointEOS &InRemoteAddr) const override;
    virtual bool HasPendingData(FSocketRedpointEOS &Socket, uint32 &PendingDataSize) const override;
    virtual bool SendTo(
        FSocketRedpointEOS &Socket,
        const uint8 *Data,
        int32 Count,
        int32 &BytesSent,
        const FInternetAddrRedpointEOS &Destination) const override;
    virtual bool RecvFrom(
        FSocketRedpointEOS &Socket,
        uint8 *Data,
        int32 BufferSize,
        int32 &BytesRead,
        FInternetAddr &Source,
        ESocketReceiveFlags::Type Flags) const override;
    virtual bool GetPeerAddress(FSocketRedpointEOS &Socket, FInternetAddrRedpointEOS &OutAddr) const override;
};

}

namespace Redpoint::EOS::Networking::Socket
{
REDPOINT_EOS_FILE_NS_EXPORT(3448274137, Redpoint::EOS::Networking::Socket, FSocketRoleClient)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()
