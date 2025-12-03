// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "RedpointEOSNetworking/Socket/SocketRoleState.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(4230421139, Redpoint::EOS::Networking::Socket, FSocketRedpointEOS)

namespace REDPOINT_EOS_FILE_NS_ID(1243964451, Redpoint::EOS::Networking::Socket)
{
using namespace ::Redpoint::EOS::Networking::Socket;

class FSocketRoleListeningState : public ISocketRoleState
{
public:
    FSocketRoleListeningState() = default;
    UE_NONCOPYABLE(FSocketRoleListeningState);
    virtual ~FSocketRoleListeningState() override = default;

    // A list of remote sockets that this listening socket owns.
    TArray<TWeakPtr<FSocketRedpointEOS>> RemoteSockets;
};

}

namespace Redpoint::EOS::Networking::Socket
{
REDPOINT_EOS_FILE_NS_EXPORT(1243964451, Redpoint::EOS::Networking::Socket, FSocketRoleListeningState)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()
