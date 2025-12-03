// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "RedpointEOSAPI/P2P/SendPacket.h"
#include "RedpointEOSAPI/SDK.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3942471015, Redpoint::EOS::Networking::Socket)
{
using namespace ::Redpoint::EOS::API::P2P;

void LogSocketPacket(const FSendPacket::Options &Opts);

}

namespace Redpoint::EOS::Networking::Socket
{
REDPOINT_EOS_FILE_NS_EXPORT(3942471015, Redpoint::EOS::Networking::Socket, LogSocketPacket)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()
