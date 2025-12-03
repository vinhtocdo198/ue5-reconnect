// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatformDesktop/RuntimePlatformWindows.h"

#include "Misc/Paths.h"
#include "RedpointEOSCore/Editor/RedpointEOSSDKInfo.h"
#include "RedpointEOSCore/Legacy/EOSString.h"
#include "RedpointEOSPlatformDesktop/Logging.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if PLATFORM_WINDOWS

namespace REDPOINT_EOS_FILE_NS_ID(2392978449, Redpoint::EOS::Platform::Desktop)
{
#if WITH_EDITOR
using namespace ::Redpoint::EOS::Core::Editor;
#endif

void FRuntimePlatformWindows::GetSearchPathsAndDllName(
#if WITH_EDITOR
    const FString &BaseDir,
#endif
    TArray<FString> &SearchPaths,
    FString &DllName)
{
#if PLATFORM_64BITS
    DllName = TEXT("EOSSDK-Win64-Shipping.dll");
#else
    DllName = TEXT("EOSSDK-Win32-Shipping.dll");
#endif

#if !WITH_EDITOR
#if PLATFORM_64BITS
    SearchPaths.Add(FPaths::ConvertRelativePathToFull(
        FPaths::ProjectDir() / TEXT("Binaries") / TEXT("Win64") / TEXT("RedpointEOS")));
#else
    SearchPaths.Add(FPaths::ConvertRelativePathToFull(
        FPaths::ProjectDir() / TEXT("Binaries") / TEXT("Win32") / TEXT("RedpointEOS")));
#endif
#else
    for (const auto &SearchPath : FRedpointEOSSDKInfo::Get().SearchPaths(TEXT("EOSSDK")))
    {
        SearchPaths.Add(FPaths::ConvertRelativePathToFull(SearchPath / TEXT("Bin")));
    }
#endif
}

void FRuntimePlatformWindows::PostLoadSuccess(const FString &BaseSearchPath)
{
#if PLATFORM_64BITS
    FString XAudio29DllName = TEXT("x64\\xaudio2_9redist.dll");
#else
    FString XAudio29DllName = TEXT("x86\\xaudio2_9redist.dll");
#endif

    // Try to find XAudio2 DLL.
    FString XAudio29DllPath = TEXT("");
    FString XAudio29DllCandidatePath = BaseSearchPath / XAudio29DllName;
    if (FPaths::FileExists(XAudio29DllCandidatePath))
    {
        XAudio29DllPath = XAudio29DllCandidatePath;
        UE_LOG(
            LogRedpointEOSPlatformDesktop,
            Verbose,
            TEXT("EOS SDK Dynamic Load - Successfully found XAudio29 DLL at: %s"),
            *XAudio29DllCandidatePath);
    }
    else
    {
        UE_LOG(
            LogRedpointEOSPlatformDesktop,
            Verbose,
            TEXT("EOS SDK Dynamic Load - Unable to locate XAudio29 DLL for Voice Chat. Expected to find in "
                 "location: %s"),
            *XAudio29DllCandidatePath);
    }

    this->WindowsRTCOptions = EOS_Windows_RTCOptions{};
    this->WindowsRTCOptions.ApiVersion = EOS_WINDOWS_RTCOPTIONS_API_LATEST;
    checkf(this->XAudio29DllPathAllocated == nullptr, TEXT("Must not have already allocated XAudio29 path!"));
    EOSString_Utf8Unlimited::AllocateToCharBuffer(XAudio29DllPath, this->XAudio29DllPathAllocated);
    this->WindowsRTCOptions.XAudio29DllPath = this->XAudio29DllPathAllocated;
}

bool FRuntimePlatformWindows::IsRTCOptionsValid() const
{
    return this->XAudio29DllPathAllocated != nullptr;
}

void *FRuntimePlatformWindows::GetRTCPlatformSpecificOptions()
{
    return &this->WindowsRTCOptions;
}

void FRuntimePlatformWindows::Unload()
{
    if (this->XAudio29DllPathAllocated != nullptr)
    {
        EOSString_Utf8Unlimited::FreeFromCharBuffer(this->XAudio29DllPathAllocated);
    }

    FRuntimePlatformDesktopBase::Unload();
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()