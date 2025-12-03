// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Logging/LogMacros.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::Networking
{

DECLARE_LOG_CATEGORY_EXTERN(LogRedpointEOSNetworking, Verbose, All);

DECLARE_LOG_CATEGORY_EXTERN(LogRedpointEOSSocket, Warning, Verbose);
DECLARE_LOG_CATEGORY_EXTERN(LogRedpointEOSSocketLifecycle, Warning, Verbose);
DECLARE_LOG_CATEGORY_EXTERN(LogRedpointEOSNetworkTrace, Verbose, All);
DECLARE_LOG_CATEGORY_EXTERN(LogRedpointEOSNetworkAuth, Verbose, All);

}

REDPOINT_EOS_CODE_GUARD_END()
