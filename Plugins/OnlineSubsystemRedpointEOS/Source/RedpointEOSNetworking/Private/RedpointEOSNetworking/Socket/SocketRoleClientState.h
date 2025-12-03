// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "RedpointEOSAPI/EventHandle.h"
#include "RedpointEOSNetworking/Socket/InternetAddrRedpointEOS.h"
#include "RedpointEOSNetworking/Socket/SocketRoleState.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(502952509, Redpoint::EOS::Networking::Socket)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Networking::Socket;

class FSocketRoleClientState : public ISocketRoleState
{
public:
    FSocketRoleClientState() = default;
    UE_NONCOPYABLE(FSocketRoleClientState);
    virtual ~FSocketRoleClientState() override = default;

    // This is the address of the remote server that this client socket is connected to.
    TSharedPtr<FInternetAddrRedpointEOS> RemoteAddr;

    // When we first connect to a channel, we need to ensure that it is "reset" on the other side. This is because
    // connections to remote players are only fully closed when all channels are closed, but we may want to perform a
    // re-connect on channel A while B stays open the whole time.
    bool bDidSendChannelReset;

    // The reset ID that we will send when we first send a packet to the remote host. This mitigates timing issues with
    // sending the reset packet.
    uint32_t SocketResetId;

    // The event handler for full connection closures.
    FEventHandlePtr OnClosedEvent;
};

}

namespace Redpoint::EOS::Networking::Socket
{
REDPOINT_EOS_FILE_NS_EXPORT(502952509, Redpoint::EOS::Networking::Socket, FSocketRoleClientState)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()
