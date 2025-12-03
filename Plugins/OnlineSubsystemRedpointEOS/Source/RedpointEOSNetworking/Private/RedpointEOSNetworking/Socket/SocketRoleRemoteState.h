// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "RedpointEOSNetworking/Socket/InternetAddrRedpointEOS.h"
#include "RedpointEOSNetworking/Socket/SocketRoleState.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(4230421139, Redpoint::EOS::Networking::Socket, FSocketRedpointEOS)

namespace REDPOINT_EOS_FILE_NS_ID(3797698006, Redpoint::EOS::Networking::Socket)
{
using namespace ::Redpoint::EOS::Networking::Socket;

class FSocketRoleRemoteState : public ISocketRoleState
{
public:
    FSocketRoleRemoteState() = default;
    UE_NONCOPYABLE(FSocketRoleRemoteState);
    virtual ~FSocketRoleRemoteState() override = default;

    // This is the address of the client that this remote socket represents.
    TSharedPtr<FInternetAddrRedpointEOS> RemoteAddr;

    // This field contains the local socket that owns it.
    TWeakPtr<FSocketRedpointEOS> RemoteSocketParent;

    // The assigned reset ID from the reset packet that was sent when this connection was established. A value of 0
    // indicates that this socket has not been assigned a reset ID yet (i.e. a normal data packet arrived first and
    // caused a new connection to be opened).
    uint32_t SocketAssignedResetId;
};

}

namespace Redpoint::EOS::Networking::Socket
{
REDPOINT_EOS_FILE_NS_EXPORT(3797698006, Redpoint::EOS::Networking::Socket, FSocketRoleRemoteState)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()
