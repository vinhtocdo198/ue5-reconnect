// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSPlatformDesktop/RuntimePlatformDesktopBase.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if PLATFORM_LINUX

namespace REDPOINT_EOS_FILE_NS_ID(526606323, Redpoint::EOS::Platform::Desktop)
{
using namespace ::Redpoint::EOS::Platform::Desktop;

class FRuntimePlatformLinux : public FRuntimePlatformDesktopBase
{
protected:
    virtual void GetSearchPathsAndDllName(
#if WITH_EDITOR
        const FString &BaseDir,
#endif
        TArray<FString> &SearchPaths,
        FString &DllName) override;

public:
    FRuntimePlatformLinux() = default;
    UE_NONCOPYABLE(FRuntimePlatformLinux);
    virtual ~FRuntimePlatformLinux() override = default;
};

}

namespace Redpoint::EOS::Platform::Desktop
{
REDPOINT_EOS_FILE_NS_EXPORT(526606323, Redpoint::EOS::Platform::Desktop, FRuntimePlatformLinux)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()