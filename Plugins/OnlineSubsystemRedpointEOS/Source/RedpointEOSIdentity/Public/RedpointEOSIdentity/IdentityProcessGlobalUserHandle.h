// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Misc/CoreMiscDefines.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1910371557, Redpoint::EOS::Identity)
{

/**
 * A handle which can be used to uniquely identity a local user in the current process across multiple platform
 * instances. This is used by the IVoiceChat APIs, which provide no other way of distinguishing between users in
 * different play-in-editor contexts.
 */
typedef FPlatformUserId FIdentityProcessGlobalUserHandle;

}

namespace Redpoint::EOS::Identity
{
REDPOINT_EOS_FILE_NS_EXPORT(1910371557, Redpoint::EOS::Identity, FIdentityProcessGlobalUserHandle)
}

REDPOINT_EOS_CODE_GUARD_END()