// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSPlatformDesktop/RuntimePlatformDesktopBase.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if PLATFORM_MAC

namespace REDPOINT_EOS_FILE_NS_ID(827953971, Redpoint::EOS::Platform::Desktop)
{
using namespace ::Redpoint::EOS::Platform::Desktop;

class FRuntimePlatformMac : public FRuntimePlatformDesktopBase
{
protected:
    virtual void GetSearchPathsAndDllName(
#if WITH_EDITOR
        const FString &BaseDir,
#endif
        TArray<FString> &SearchPaths,
        FString &DllName) override;

public:
    FRuntimePlatformMac() = default;
    UE_NONCOPYABLE(FRuntimePlatformMac);
    virtual ~FRuntimePlatformMac() override = default;
};

}

namespace Redpoint::EOS::Platform::Desktop
{
REDPOINT_EOS_FILE_NS_EXPORT(827953971, Redpoint::EOS::Platform::Desktop, FRuntimePlatformMac)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()