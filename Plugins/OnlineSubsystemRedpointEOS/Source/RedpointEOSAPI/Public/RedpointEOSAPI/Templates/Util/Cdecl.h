// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if PLATFORM_ANDROID
#define __REDPOINT_EOSSDK_CDECL_ATTR
#else
#define __REDPOINT_EOSSDK_CDECL_ATTR __cdecl
#endif

REDPOINT_EOS_CODE_GUARD_END()