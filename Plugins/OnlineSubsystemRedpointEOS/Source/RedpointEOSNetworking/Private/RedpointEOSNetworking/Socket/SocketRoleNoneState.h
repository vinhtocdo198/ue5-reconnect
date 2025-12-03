// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "RedpointEOSNetworking/Socket/SocketRoleState.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(332633705, Redpoint::EOS::Networking::Socket)
{
using namespace ::Redpoint::EOS::Networking::Socket;

class FSocketRoleNoneState : public ISocketRoleState
{
public:
    FSocketRoleNoneState() = default;
    UE_NONCOPYABLE(FSocketRoleNoneState);
    virtual ~FSocketRoleNoneState() override = default;
};

}

namespace Redpoint::EOS::Networking::Socket
{
REDPOINT_EOS_FILE_NS_EXPORT(332633705, Redpoint::EOS::Networking::Socket, FSocketRoleNoneState)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()
