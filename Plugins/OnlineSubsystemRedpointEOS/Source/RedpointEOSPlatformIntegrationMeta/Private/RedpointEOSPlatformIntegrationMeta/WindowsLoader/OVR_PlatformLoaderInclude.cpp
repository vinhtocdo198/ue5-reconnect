// Copyright June Rhodes. All Rights Reserved.

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_META_ENABLED && defined(PLATFORM_WINDOWS) && PLATFORM_WINDOWS

#include "Windows/AllowWindowsPlatformTypes.h"

#include "Windows/WindowsHWrapper.h"

#if !REDPOINT_EOS_META_USE_REDPOINT_PLATFORM_LOADER
// This file is located in the OVRPlatformSDK and is added to the include path by the build system.
#include "OVR_PlatformLoader.cpp"
#else
// This file is located in this folder, and is necessary on older versions of Unreal Engine that don't
// ship the complete Oculus Platform SDK needed for Windows.
#include "RedpointEOSPlatformIntegrationMeta/WindowsLoader/OVR_PlatformLoader.h"
#endif

#include "Windows/HideWindowsPlatformTypes.h"

#endif