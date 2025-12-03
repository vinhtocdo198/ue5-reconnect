// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatformDesktop/RuntimePlatformDesktopBase.h"

#include "GenericPlatform/GenericPlatformFile.h"
#include "HAL/PlatformFileManager.h"
#include "HAL/PlatformProcess.h"
#include "Interfaces/IPluginManager.h"
#include "Logging/LogMacros.h"
#include "Misc/App.h"
#include "Misc/Paths.h"
#include "RedpointEOSCore/Legacy/EOSString.h"
#include "RedpointEOSPlatformDesktop/Logging.h"
#if !WITH_EDITOR
#include "Misc/MessageDialog.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1340308852, Redpoint::EOS::Platform::Desktop)
{
using namespace ::Redpoint::EOS::Platform::Desktop;

void FRuntimePlatformDesktopBase::PostLoadSuccess(const FString &BaseSearchPath)
{
}

bool FRuntimePlatformDesktopBase::IsRTCOptionsValid() const
{
    return true;
}

void *FRuntimePlatformDesktopBase::GetRTCPlatformSpecificOptions()
{
    return nullptr;
}

void FRuntimePlatformDesktopBase::Load()
{
    UE_LOG(LogRedpointEOSPlatformDesktop, Verbose, TEXT("EOS SDK Dynamic Load - Starting dynamic load of EOS SDK..."));

    // Determine the library search paths for this platform.
    TArray<FString> SearchPaths;
#if WITH_EDITOR
    auto LocatedPlugin = IPluginManager::Get().FindPlugin("OnlineSubsystemRedpointEOS");
    checkf(
        LocatedPlugin.IsValid(),
        TEXT("Expected to be able to locate the 'OnlineSubsystemRedpointEOS' plugin when running in the editor. This "
             "is required in order to locate the EOS SDK to load."));
    FString BaseDir = LocatedPlugin->GetBaseDir();
#endif
    FString DllName;
    this->GetSearchPathsAndDllName(
#if WITH_EDITOR
        BaseDir,
#endif
        SearchPaths,
        DllName);

    bool bSimulateDLLNotFound =
        FString(FPlatformMisc::GetEnvironmentVariable(TEXT("EOS_SIMULATE_DLL_NOT_AVAILABLE"))) == TEXT("true");
    if (bSimulateDLLNotFound)
    {
        // We must delete the EOS SDK DLL if it resides in the default Windows search path, otherwise the DLL will
        // be implicitly loaded by the linker even if we fail to load it here.
        FString WindowsDllPath =
            FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() / TEXT("Binaries") / TEXT("Win64") / DllName);
        if (FPaths::FileExists(WindowsDllPath))
        {
            UE_LOG(
                LogRedpointEOSPlatformDesktop,
                Warning,
                TEXT("EOS SDK Dynamic Load - Removing EOS SDK DLL at this path to ensure that "
                     "EOS_SIMULATE_DLL_NOT_AVAILABLE=true acts as intended: %s"),
                *WindowsDllPath);
            FPlatformFileManager::Get().GetPlatformFile().DeleteFile(*WindowsDllPath);
        }
    }

    // Now try each of them in sequence.
    for (const auto &BaseSearchPath : SearchPaths)
    {
        FString SearchPath = BaseSearchPath / DllName;

        if (bSimulateDLLNotFound)
        {
            UE_LOG(
                LogRedpointEOSPlatformDesktop,
                Warning,
                TEXT("EOS SDK Dynamic Load - Not checking path because environment variable "
                     "EOS_SIMULATE_DLL_NOT_AVAILABLE=true: %s"),
                *SearchPath);
            continue;
        }

        if (!FPaths::FileExists(SearchPath))
        {
            UE_LOG(
                LogRedpointEOSPlatformDesktop,
                Verbose,
                TEXT("EOS SDK Dynamic Load - Could not locate the DLL at: %s"),
                *SearchPath);
            continue;
        }

        UE_LOG(LogRedpointEOSPlatformDesktop, Verbose, TEXT("EOS SDK Dynamic Load - DLL found at: %s"), *SearchPath);
        FPlatformProcess::PushDllDirectory(*FPaths::GetPath(SearchPath));
        this->DynamicLibraryHandle = FPlatformProcess::GetDllHandle(*SearchPath);
        FPlatformProcess::PopDllDirectory(*FPaths::GetPath(SearchPath));

        if (this->DynamicLibraryHandle != nullptr)
        {
            UE_LOG(
                LogRedpointEOSPlatformDesktop,
                Verbose,
                TEXT("EOS SDK Dynamic Load - Successfully loaded DLL at: %s"),
                *SearchPath);
            this->PostLoadSuccess(BaseSearchPath);
            break;
        }
        else
        {
            UE_LOG(
                LogRedpointEOSPlatformDesktop,
                Error,
                TEXT("EOS SDK Dynamic Load - Failed to load DLL at: %s"),
                *SearchPath);
        }
    }

    if (this->DynamicLibraryHandle == nullptr)
    {
        UE_LOG(
            LogRedpointEOSPlatformDesktop,
            Error,
            TEXT("EOS SDK Dynamic Load - Unable to locate the EOS SDK DLL in any of the supported locations. Check the "
                 "documentation for instructions on how to install the SDK."));
#if !WITH_EDITOR
        FMessageDialog::Open(
            EAppMsgType::Ok,
            NSLOCTEXT(
                "FOnlineSubsystemRedpointEOSModule",
                "OnlineSubsystemEOS_SDKNotFound",
                "The EOS SDK could not be found. Please reinstall the application."));
        FPlatformMisc::RequestExit(false);
#endif
        return;
    }

    this->RTCOptions = EOS_Platform_RTCOptions{};
    this->RTCOptions.ApiVersion = EOS_PLATFORM_RTCOPTIONS_API_LATEST;
    this->RTCOptions.PlatformSpecificOptions = this->GetRTCPlatformSpecificOptions();
}

void FRuntimePlatformDesktopBase::Unload()
{
    UE_LOG(
        LogRedpointEOSPlatformDesktop,
        Verbose,
        TEXT("EOS SDK Dynamic Unload - Starting dynamic unload of EOS SDK..."));
    if (this->DynamicLibraryHandle != nullptr)
    {
        FPlatformProcess::FreeDllHandle(this->DynamicLibraryHandle);
    }
    this->DynamicLibraryHandle = nullptr;
}

bool FRuntimePlatformDesktopBase::IsValid()
{
    return this->DynamicLibraryHandle != nullptr;
}

void *FRuntimePlatformDesktopBase::GetSystemInitializeOptions()
{
    return nullptr;
}

EOS_Platform_RTCOptions *FRuntimePlatformDesktopBase::GetRTCOptions()
{
    if (!this->IsRTCOptionsValid())
    {
        return nullptr;
    }

    return &this->RTCOptions;
}

FString FRuntimePlatformDesktopBase::GetCacheDirectory()
{
    FString Path = FPaths::ProjectPersistentDownloadDir() / TEXT("EOSCache");
    Path = FPaths::ConvertRelativePathToFull(Path);

#if PLATFORM_WINDOWS
    // If our save directory is on a UNC share, disable the cache directory
    // since EOS can't work with UNC paths. The only supported scenario for
    // a save directory on a UNC share is for the Anti-Cheat Gauntlet tests,
    // which don't require the cache to operate anyway.
    if (Path.StartsWith(TEXT("//")))
    {
        UE_LOG(
            LogRedpointEOSPlatformDesktop,
            Warning,
            TEXT("EOS cache will be turned off, as this game is running with a persistence directory located on a UNC "
                 "share. This will prevent Player Data Storage and Title File services from working."));
        return TEXT("");
    }
#endif

    UE_LOG(LogRedpointEOSPlatformDesktop, Verbose, TEXT("Using the following path as the cache directory: %s"), *Path);
    IPlatformFile &PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    if (!PlatformFile.DirectoryExists(*Path))
    {
        PlatformFile.CreateDirectory(*Path);
    }
    return Path;
}

#if !UE_BUILD_SHIPPING
FString FRuntimePlatformDesktopBase::GetPathToEASAutomatedTestingCredentials()
{
    return TEXT("");
}
#endif

bool FRuntimePlatformDesktopBase::UseCrossPlatformFriendStorage() const
{
    return true;
}

#if !defined(UE_SERVER) || !UE_SERVER

bool FRuntimePlatformDesktopBase::ShouldPollLifecycleApplicationStatus() const
{
    return true;
}

bool FRuntimePlatformDesktopBase::ShouldPollLifecycleNetworkStatus() const
{
    return false;
}

bool FRuntimePlatformDesktopBase::PollLifecycleApplicationStatus(EOS_EApplicationStatus &OutApplicationStatus) const
{
    // Desktop platforms always run applications in the foreground, regardless of whether the
    // application is the active focus.
    OutApplicationStatus = EOS_EApplicationStatus::EOS_AS_Foreground;
    return true;
}

bool FRuntimePlatformDesktopBase::PollLifecycleNetworkStatus(EOS_ENetworkStatus &OutNetworkStatus) const
{
    return false;
}

#else

void FRuntimePlatformDesktopBase::SetLifecycleForNewPlatformInstance(Redpoint::EOS::API::FPlatformHandle InPlatform)
{
    EOS_Platform_SetApplicationStatus(InPlatform->Handle(), EOS_EApplicationStatus::EOS_AS_Foreground);
    EOS_Platform_SetNetworkStatus(InPlatform->Handle(), EOS_ENetworkStatus::EOS_NS_Online);
}

#endif

}

REDPOINT_EOS_CODE_GUARD_END()