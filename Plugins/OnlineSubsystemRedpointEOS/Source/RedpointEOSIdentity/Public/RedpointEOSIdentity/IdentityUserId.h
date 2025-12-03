// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Online/CoreOnline.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2603205178, Redpoint::EOS::Identity)
{

/**
 * A handle which represents the user ID of a locally signed in user or remote user. Resolving this user ID to a string
 * will give you the PUID of a user.
 */
typedef UE::Online::FAccountId FIdentityUserId;

}

namespace Redpoint::EOS::Identity
{
REDPOINT_EOS_FILE_NS_EXPORT(2603205178, Redpoint::EOS::Identity, FIdentityUserId)
}

REDPOINT_EOS_CODE_GUARD_END()