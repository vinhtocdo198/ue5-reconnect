// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

namespace Redpoint::EOS::Core
{

// @note: This is exported because FMultiOperation is a template which uses this log category, and thus it must be
// exported so the generated code in other modules can reference it.
REDPOINTEOSCORE_API DECLARE_LOG_CATEGORY_EXTERN(LogRedpointEOSCore, Verbose, All);

}

// @note: This has moved from OnlineSubsystemRedpointEOS because RedpointEOSCore
// now handles forwarding the EOS SDK logs and thus we need to use it in this module.
REDPOINTEOSCORE_API DECLARE_LOG_CATEGORY_EXTERN(LogRedpointEOS, Verbose, All);