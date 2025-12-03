// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSIdentity/Stats.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::Identity
{

REDPOINT_EOS_DEFINE_STAT(Identity, Login);
REDPOINT_EOS_DEFINE_STAT(Identity, Logout);
REDPOINT_EOS_DEFINE_STAT(Identity, UnexpectedLogout);

}

REDPOINT_EOS_CODE_GUARD_END()
