// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSPlatformDesktop/RuntimePlatformDesktopBase.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if PLATFORM_WINDOWS

namespace REDPOINT_EOS_FILE_NS_ID(2392978449, Redpoint::EOS::Platform::Desktop)
{
using namespace ::Redpoint::EOS::Platform::Desktop;

class FRuntimePlatformWindows : public FRuntimePlatformDesktopBase
{
private:
    EOS_Windows_RTCOptions WindowsRTCOptions;
    const char *XAudio29DllPathAllocated;

protected:
    virtual void GetSearchPathsAndDllName(
#if WITH_EDITOR
        const FString &BaseDir,
#endif
        TArray<FString> &SearchPaths,
        FString &DllName) override;
    virtual void PostLoadSuccess(const FString &BaseSearchPath) override;
    virtual bool IsRTCOptionsValid() const override;
    virtual void *GetRTCPlatformSpecificOptions() override;

public:
    FRuntimePlatformWindows() = default;
    UE_NONCOPYABLE(FRuntimePlatformWindows);
    virtual ~FRuntimePlatformWindows() override = default;

    virtual void Unload() override;
};

}

namespace Redpoint::EOS::Platform::Desktop
{
REDPOINT_EOS_FILE_NS_EXPORT(2392978449, Redpoint::EOS::Platform::Desktop, FRuntimePlatformWindows)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()