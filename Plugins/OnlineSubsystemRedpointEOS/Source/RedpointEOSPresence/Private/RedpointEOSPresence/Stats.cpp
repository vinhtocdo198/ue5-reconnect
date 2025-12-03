// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPresence/Stats.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace Redpoint::EOS::Presence
{

REDPOINT_EOS_DEFINE_STAT(Presence, OnProcessRequestedUpdateGraph);

}

#endif

REDPOINT_EOS_CODE_GUARD_END()
