// Copyright June Rhodes. All Rights Reserved.

#pragma once

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_APPLE_ENABLED

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::Platform::Integration::Apple
{

DECLARE_LOG_CATEGORY_EXTERN(LogRedpointEOSPlatformIntegrationApple, Verbose, All);

}

REDPOINT_EOS_CODE_GUARD_END()

#endif
