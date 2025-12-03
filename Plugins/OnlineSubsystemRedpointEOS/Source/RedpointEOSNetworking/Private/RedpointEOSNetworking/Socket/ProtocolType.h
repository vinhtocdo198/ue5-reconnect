// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(2992644456, Redpoint::EOS::Networking::Socket)
{

extern const FName ProtocolType;

}

namespace Redpoint::EOS::Networking::Socket
{
REDPOINT_EOS_FILE_NS_EXPORT(2992644456, Redpoint::EOS::Networking::Socket, ProtocolType)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()