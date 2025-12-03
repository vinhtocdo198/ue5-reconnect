// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3250394593, Redpoint::EOS::Networking)
{

enum ENetDriverRole : int8
{
    DedicatedServer,
    ListenServer,
    ClientConnectedToDedicatedServer,
    ClientConnectedToListenServer,
};

}

namespace Redpoint::EOS::Networking
{
REDPOINT_EOS_FILE_NS_EXPORT(3250394593, Redpoint::EOS::Networking, ENetDriverRole)
}

REDPOINT_EOS_CODE_GUARD_END()