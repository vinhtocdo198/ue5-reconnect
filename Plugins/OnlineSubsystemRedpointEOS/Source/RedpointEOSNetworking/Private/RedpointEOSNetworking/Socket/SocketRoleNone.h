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

namespace REDPOINT_EOS_FILE_NS_ID(4168139954, Redpoint::EOS::Networking::Socket)
{
using namespace ::Redpoint::EOS::Networking::Socket;

/**
 * The None role is the initial role for newly created sockets, and allows them to move into either Client or Listening.
 */
class FSocketRoleNone : public ISocketRole
{
public:
    FSocketRoleNone() = default;
    UE_NONCOPYABLE(FSocketRoleNone);
    virtual ~FSocketRoleNone() override = default;

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
REDPOINT_EOS_FILE_NS_EXPORT(4168139954, Redpoint::EOS::Networking::Socket, FSocketRoleNone)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()
