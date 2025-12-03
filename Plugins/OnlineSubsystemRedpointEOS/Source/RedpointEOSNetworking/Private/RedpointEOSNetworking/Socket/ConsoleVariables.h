// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "HAL/IConsoleManager.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && !(defined(UE_BUILD_SHIPPING) && UE_BUILD_SHIPPING)

namespace REDPOINT_EOS_FILE_NS_ID(3821566149, Redpoint::EOS::Networking::Socket)
{

extern TAutoConsoleVariable<bool> CVarOnDemandPacketDispatch;
extern TAutoConsoleVariable<bool> CVarEnablePacketTiming;

}

namespace Redpoint::EOS::Networking::Socket
{
REDPOINT_EOS_FILE_NS_EXPORT(3821566149, Redpoint::EOS::Networking::Socket, CVarOnDemandPacketDispatch)
REDPOINT_EOS_FILE_NS_EXPORT(3821566149, Redpoint::EOS::Networking::Socket, CVarEnablePacketTiming)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()