// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(698769264, Redpoint::EOS::Networking::Socket)
{

class ISocketRoleState : public TSharedFromThis<ISocketRoleState>
{
public:
    ISocketRoleState() = default;
    UE_NONCOPYABLE(ISocketRoleState);
    virtual ~ISocketRoleState() = default;
};

}

namespace Redpoint::EOS::Networking::Socket
{
REDPOINT_EOS_FILE_NS_EXPORT(698769264, Redpoint::EOS::Networking::Socket, ISocketRoleState)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()
