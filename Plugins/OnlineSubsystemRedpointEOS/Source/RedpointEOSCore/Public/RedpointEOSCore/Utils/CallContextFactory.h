// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSPlatform/Services/RuntimePlatformServiceCallContext.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(178046447, Redpoint::EOS::Core::Utils)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Platform::Services;

REDPOINTEOSCORE_API FRuntimePlatformServiceCallContextRef
ConstructRuntimePlatformServiceCallContext(const FPlatformHandle &PlatformHandle, int32 LocalUserNum);

}

namespace Redpoint::EOS::Core::Utils
{
REDPOINT_EOS_FILE_NS_EXPORT(178046447, Redpoint::EOS::Core::Utils, ConstructRuntimePlatformServiceCallContext)
}

REDPOINT_EOS_CODE_GUARD_END()