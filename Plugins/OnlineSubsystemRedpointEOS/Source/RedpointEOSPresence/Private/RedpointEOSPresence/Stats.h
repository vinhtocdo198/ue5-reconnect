// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Stats.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace Redpoint::EOS::Presence
{

REDPOINT_EOS_DECLARE_CYCLE_STAT(Presence, TEXT("OnProcessRequestedUpdateGraph"), OnProcessRequestedUpdateGraph);

}

#endif

REDPOINT_EOS_CODE_GUARD_END()
