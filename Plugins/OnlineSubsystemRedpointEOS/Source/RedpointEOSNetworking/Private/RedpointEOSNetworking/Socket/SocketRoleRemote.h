// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "RedpointEOSNetworking/Socket/InternetAddrRedpointEOS.h"
#include "RedpointEOSNetworking/Socket/SocketRole.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(4230421139, Redpoint::EOS::Networking::Socket, FSocketRedpointEOS)

namespace REDPOINT_EOS_FILE_NS_ID(767262975, Redpoint::EOS::Networking::Socket)
{
using namespace ::Redpoint::EOS::Networking::Socket;

/**
 * Remote sockets are created and managed by Listening sockets, and they represent remote sockets in the Client role.
 * They only ever communicate with the client they are associated with.
 */
class FSocketRoleRemote : public ISocketRole
{
public:
    FSocketRoleRemote() = default;
    UE_NONCOPYABLE(FSocketRoleRemote);
    virtual ~FSocketRoleRemote() override = default;

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
REDPOINT_EOS_FILE_NS_EXPORT(767262975, Redpoint::EOS::Networking::Socket, FSocketRoleRemote)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()
