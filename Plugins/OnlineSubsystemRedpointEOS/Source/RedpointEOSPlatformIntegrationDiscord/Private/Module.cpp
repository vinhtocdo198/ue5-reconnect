// Copyright June Rhodes. All Rights Reserved.

#include "Modules/ModuleManager.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_DISCORD_ENABLED
#include "Containers/Ticker.h"
#include "HAL/PlatformProcess.h"
#include "Misc/Paths.h"
#include "RedpointEOSCore/Editor/RedpointEOSSDKInfo.h"
#include "RedpointEOSCore/Module.h"
#include "RedpointEOSPlatformIntegrationDiscord/Logging.h"
#include "RedpointEOSPlatformIntegrationDiscord/RuntimePlatformIntegrationDiscord.h"
#include "discordpp.h"
#endif

class REDPOINTEOSPLATFORMINTEGRATIONDISCORD_API FRedpointEOSPlatformIntegrationDiscordModule : public FDefaultModuleImpl
{
#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_DISCORD_ENABLED
private:
    void *DynamicLibraryHandle;
    FTSTicker::FDelegateHandle TickHandle;

public:
    virtual void StartupModule() override
    {
        using namespace ::Redpoint::EOS::Core;
#if WITH_EDITOR
        using namespace ::Redpoint::EOS::Core::Editor;
#endif
        using namespace ::Redpoint::EOS::Platform::Integration::Discord;

        // Determine paths that we will search for the Discord runtime library under.
        TArray<FString> SearchPaths;
#if WITH_EDITOR
        for (const auto &SearchPath : FRedpointEOSSDKInfo::Get().SearchPaths(TEXT("DiscordSocialSDK")))
        {
            SearchPaths.Add(FPaths::ConvertRelativePathToFull(SearchPath / TEXT("bin") / TEXT("release")));
            SearchPaths.Add(FPaths::ConvertRelativePathToFull(SearchPath / TEXT("lib") / TEXT("release")));
        }
#elif PLATFORM_WINDOWS
        SearchPaths.Add(FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() / TEXT("Binaries") / TEXT("Win64")));
#elif PLATFORM_MAC
        // Due to macOS RPATH shenanigans, packaged games need the library next to the main executable in
        // /Contents/MacOS/, not in the /Contents/UE4/<project>/Binaries/Mac/ inside the bundle.
        SearchPaths.Add(
            FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() / TEXT("..") / TEXT("..") / TEXT("MacOS")));

        // When executing staged builds under Gauntlet, the game is not a packaged build, and the path to the SDK is
        // different.
        SearchPaths.Add(FPaths::ConvertRelativePathToFull(
            FPaths::ProjectDir() / TEXT("Binaries") / TEXT("Mac") /
            FString::Printf(TEXT("%s-Mac-Development.app"), FApp::GetProjectName()) / TEXT("Contents") /
            TEXT("MacOS")));
        SearchPaths.Add(FPaths::ConvertRelativePathToFull(
            FPaths::ProjectDir() / TEXT("Binaries") / TEXT("Mac") /
            FString::Printf(TEXT("%s-Mac-DebugGame.app"), FApp::GetProjectName()) / TEXT("Contents") / TEXT("MacOS")));
#elif PLATFORM_LINUXARM64
#error Linux ARM64 is not supported for the Discord Social SDK.
#elif PLATFORM_LINUX
        SearchPaths.Add(FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() / TEXT("Binaries") / TEXT("Linux")));
#else
#error This platform is not yet supported when enabling the Discord Social SDK.
#endif

        // Determine the name of the runtime library.
#if PLATFORM_WINDOWS
        FString DllName = TEXT("discord_partner_sdk.dll");
#elif PLATFORM_MAC
        FString DllName = TEXT("libdiscord_partner_sdk.dylib");
#elif PLATFORM_LINUX
        FString DllName = TEXT("libdiscord_partner_sdk.so");
#endif

        // Attempt to load the runtime library.
        for (const auto &BaseSearchPath : SearchPaths)
        {
            FString SearchPath = BaseSearchPath / DllName;

            if (!FPaths::FileExists(SearchPath))
            {
                UE_LOG(
                    LogRedpointEOSPlatformIntegrationDiscord,
                    Verbose,
                    TEXT("Discord Social SDK Dynamic Load - Could not locate the DLL at: %s"),
                    *SearchPath);
                continue;
            }

            UE_LOG(
                LogRedpointEOSPlatformIntegrationDiscord,
                Verbose,
                TEXT("Discord Social SDK Dynamic Load - DLL found at: %s"),
                *SearchPath);
            FPlatformProcess::PushDllDirectory(*FPaths::GetPath(SearchPath));
            this->DynamicLibraryHandle = FPlatformProcess::GetDllHandle(*SearchPath);
            FPlatformProcess::PopDllDirectory(*FPaths::GetPath(SearchPath));

            if (this->DynamicLibraryHandle != nullptr)
            {
                UE_LOG(
                    LogRedpointEOSPlatformIntegrationDiscord,
                    Verbose,
                    TEXT("Discord Social SDK Dynamic Load - Successfully loaded DLL at: %s"),
                    *SearchPath);
                break;
            }
            else
            {
                UE_LOG(
                    LogRedpointEOSPlatformIntegrationDiscord,
                    Error,
                    TEXT("Discord Social SDK Dynamic Load - Failed to load DLL at: %s"),
                    *SearchPath);
            }
        }

        // If the dynamic library handle is still nullptr, we don't have Discord integration at runtime.
        if (this->DynamicLibraryHandle == nullptr)
        {
            UE_LOG(
                LogRedpointEOSPlatformIntegrationDiscord,
                Error,
                TEXT("Discord Social SDK integration will not be available, as we could not locate the runtime "
                     "library."));
        }
        else
        {
            UE_LOG(
                LogRedpointEOSPlatformIntegrationDiscord,
                Verbose,
                TEXT("Discord Social SDK integration successfully loaded."));

            FModule::GetModuleChecked().GetRuntimePlatformRegistry()->AddRuntimePlatformIntegration(
                FName(TEXT("Discord")),
                MakeShared<FRuntimePlatformIntegrationDiscord>());

            this->TickHandle = FTSTicker::GetCoreTicker().AddTicker(
                FTickerDelegate::CreateLambda([](float) -> bool {
                    discordpp::RunCallbacks();
                    return true;
                }),
                0.0f);
        }
    }

    virtual void ShutdownModule() override
    {
        using namespace ::Redpoint::EOS::Core;
        using namespace ::Redpoint::EOS::Platform::Integration::Discord;

        if (this->DynamicLibraryHandle != nullptr)
        {
            FTSTicker::GetCoreTicker().RemoveTicker(this->TickHandle);

            FModule::GetModuleChecked().GetRuntimePlatformRegistry()->RemoveRuntimePlatformIntegration(
                FName(TEXT("Discord")));

            UE_LOG(
                LogRedpointEOSPlatformIntegrationDiscord,
                Verbose,
                TEXT("Discord Social SDK Dynamic Unload - Starting dynamic unload of EOS SDK..."));

            if (this->DynamicLibraryHandle != nullptr)
            {
                FPlatformProcess::FreeDllHandle(this->DynamicLibraryHandle);
            }
            this->DynamicLibraryHandle = nullptr;

            UE_LOG(
                LogRedpointEOSPlatformIntegrationDiscord,
                Verbose,
                TEXT("Discord Social SDK integration successfully unloaded."));
        }
    }
#endif
};

IMPLEMENT_MODULE(FRedpointEOSPlatformIntegrationDiscordModule, RedpointEOSPlatformIntegrationDiscord);