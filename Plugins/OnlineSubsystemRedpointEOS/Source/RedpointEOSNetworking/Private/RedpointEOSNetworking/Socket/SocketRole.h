// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "RedpointEOSNetworking/Socket/SocketRedpointEOS.h"
#include "Sockets.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(3518527788, Redpoint::EOS::Networking::Socket, FInternetAddrRedpointEOS)

namespace REDPOINT_EOS_FILE_NS_ID(1057720780, Redpoint::EOS::Networking::Socket)
{
using namespace ::Redpoint::EOS::Networking::Socket;

class ISocketRole : public TSharedFromThis<ISocketRole>
{
protected:
    template <typename TState> TSharedRef<TState> GetState(FSocketRedpointEOS &Socket) const
    {
        return StaticCastSharedRef<TState>(Socket.RoleState);
    }

public:
    ISocketRole() = default;
    UE_NONCOPYABLE(ISocketRole);
    virtual ~ISocketRole() = default;

    virtual FName GetRoleName() const = 0;

    virtual bool Close(FSocketRedpointEOS &Socket) const = 0;
    virtual bool Connect(FSocketRedpointEOS &Socket, const FInternetAddrRedpointEOS &InDestAddr) const = 0;
    virtual bool Listen(FSocketRedpointEOS &Socket, int32 MaxBacklog) const = 0;
    virtual bool Accept(
        FSocketRedpointEOS &Socket,
        TSharedRef<FSocketRedpointEOS> InRemoteSocket,
        const FInternetAddrRedpointEOS &InRemoteAddr) const = 0;
    virtual bool HasPendingData(FSocketRedpointEOS &Socket, uint32 &PendingDataSize) const = 0;
    virtual bool SendTo(
        FSocketRedpointEOS &Socket,
        const uint8 *Data,
        int32 Count,
        int32 &BytesSent,
        const FInternetAddrRedpointEOS &Destination) const = 0;
    virtual bool RecvFrom(
        FSocketRedpointEOS &Socket,
        uint8 *Data,
        int32 BufferSize,
        int32 &BytesRead,
        FInternetAddr &Source,
        ESocketReceiveFlags::Type Flags) const = 0;
    virtual bool GetPeerAddress(FSocketRedpointEOS &Socket, FInternetAddrRedpointEOS &OutAddr) const = 0;

    virtual const TArray<TWeakPtr<FSocketRedpointEOS>> GetOwnedSockets(FSocketRedpointEOS &Socket) const
    {
        return TArray<TWeakPtr<FSocketRedpointEOS>>();
    };
};

}

namespace Redpoint::EOS::Networking::Socket
{
REDPOINT_EOS_FILE_NS_EXPORT(1057720780, Redpoint::EOS::Networking::Socket, ISocketRole)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()
