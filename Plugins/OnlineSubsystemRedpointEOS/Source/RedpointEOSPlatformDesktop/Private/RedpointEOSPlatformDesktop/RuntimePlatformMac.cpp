// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatformDesktop/RuntimePlatformMac.h"

#include "Misc/App.h"
#include "Misc/Paths.h"
#include "RedpointEOSCore/Editor/RedpointEOSSDKInfo.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if PLATFORM_MAC

namespace REDPOINT_EOS_FILE_NS_ID(827953971, Redpoint::EOS::Platform::Desktop)
{
#if WITH_EDITOR
using namespace ::Redpoint::EOS::Core::Editor;
#endif

void FRuntimePlatformMac::GetSearchPathsAndDllName(
#if WITH_EDITOR
    const FString &BaseDir,
#endif
    TArray<FString> &SearchPaths,
    FString &DllName)
{
    FString Home = FPlatformMisc::GetEnvironmentVariable(TEXT("HOME"));

    DllName = TEXT("libEOSSDK-Mac-Shipping.dylib");

#if !WITH_EDITOR
    // Due to macOS RPATH shenanigans, packaged games need the library next to the main executable in /Contents/MacOS/,
    // not in the /Contents/UE4/<project>/Binaries/Mac/ inside the bundle.
    SearchPaths.Add(FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() / TEXT("..") / TEXT("..") / TEXT("MacOS")));

    // When executing staged builds under Gauntlet, the game is not a packaged build, and the path to the SDK is
    // different.
    SearchPaths.Add(FPaths::ConvertRelativePathToFull(
        FPaths::ProjectDir() / TEXT("Binaries") / TEXT("Mac") /
        FString::Printf(TEXT("%s-Mac-Development.app"), FApp::GetProjectName()) / TEXT("Contents") / TEXT("MacOS")));
    SearchPaths.Add(FPaths::ConvertRelativePathToFull(
        FPaths::ProjectDir() / TEXT("Binaries") / TEXT("Mac") /
        FString::Printf(TEXT("%s-Mac-DebugGame.app"), FApp::GetProjectName()) / TEXT("Contents") / TEXT("MacOS")));
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