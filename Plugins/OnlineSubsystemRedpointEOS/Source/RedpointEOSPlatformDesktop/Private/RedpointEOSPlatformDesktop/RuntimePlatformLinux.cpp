// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatformDesktop/RuntimePlatformLinux.h"

#include "Misc/Paths.h"
#include "RedpointEOSCore/Editor/RedpointEOSSDKInfo.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if PLATFORM_LINUX

namespace REDPOINT_EOS_FILE_NS_ID(526606323, Redpoint::EOS::Platform::Desktop)
{
#if WITH_EDITOR
using namespace ::Redpoint::EOS::Core::Editor;
#endif

void FRuntimePlatformLinux::GetSearchPathsAndDllName(
#if WITH_EDITOR
    const FString &BaseDir,
#endif
    TArray<FString> &SearchPaths,
    FString &DllName)
{
    FString Home = FPlatformMisc::GetEnvironmentVariable(TEXT("HOME"));

#if PLATFORM_LINUXARM64
    DllName = TEXT("libEOSSDK-LinuxArm64-Shipping.so");
#else
    DllName = TEXT("libEOSSDK-Linux-Shipping.so");
#endif

#if !WITH_EDITOR
#if PLATFORM_LINUXARM64
    SearchPaths.Add(FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() / TEXT("Binaries") / TEXT("LinuxArm64")));
#else
    SearchPaths.Add(FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() / TEXT("Binaries") / TEXT("Linux")));
#endif
#else
    for (const auto &SearchPath : FRedpointEOSSDKInfo::Get().SearchPaths(TEXT("EOSSDK")))
    {
        SearchPaths.Add(FPaths::ConvertRelativePathToFull(SearchPath / TEXT("Bin")));
    }
#endif
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()