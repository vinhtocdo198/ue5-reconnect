// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSConfig/DependentModuleLoader.h"

#include "Modules/ModuleManager.h"
#include "RedpointEOSConfig/Logging.h"

namespace Redpoint::EOS::Config
{

void FDependentModuleLoader::LoadConfigDependentModules(IConfig &InConfig)
{
    FModuleManager &ModuleManager = FModuleManager::Get();

    // Load modules that are needed by delegated subsystems.
    TArray<FString> SubsystemList;
    InConfig.GetDelegatedSubsystemsString().ParseIntoArray(SubsystemList, TEXT(","), true);
    for (const auto &SubsystemName : SubsystemList)
    {
        FString ModuleName = FString::Printf(TEXT("OnlineSubsystem%s"), *SubsystemName);
        auto Module = ModuleManager.LoadModule(FName(*ModuleName));
        if (Module == nullptr)
        {
            UE_LOG(
                LogRedpointEOSConfig,
                Warning,
                TEXT("Unable to load module for delegated subsystem: %s"),
                *ModuleName);
        }
    }
}

void FDependentModuleLoader::LoadPlatformDependentModules()
{
    FModuleManager &ModuleManager = FModuleManager::Get();

    // Try to load the platform extension module for the current platform.
    {
        FName ModuleName = NAME_None;
        bool bIsPrivatePlatform = false;
        // @note: "!PLATFORM_IS_EXTENSION" is required here, as some confidential platforms also define e.g.
        // PLATFORM_WINDOWS to be true. Confidential platforms can never make use of the public RedpointEOSPlatform
        // module names listed here and must always fallthrough to the confidential platform module name.
#if !PLATFORM_IS_EXTENSION && (PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX)
        ModuleName = FName("RedpointEOSPlatformDesktop");
#elif !PLATFORM_IS_EXTENSION && (PLATFORM_IOS)
        ModuleName = FName("RedpointEOSPlatformIOS");
#elif !PLATFORM_IS_EXTENSION && (PLATFORM_TVOS)
#error The TVOS platform is not supported by the EOS SDK, and the RedpointEOSConfig module should not be compiling for it.
#elif !PLATFORM_IS_EXTENSION && (PLATFORM_ANDROID)
        ModuleName = FName("RedpointEOSPlatformAndroid");
#else
        ModuleName = FName(PREPROCESSOR_TO_STRING(PREPROCESSOR_JOIN(RedpointEOSPlatform, PLATFORM_HEADER_NAME)));
        bIsPrivatePlatform = true;
#endif
        UE_LOG(LogRedpointEOSConfig, Verbose, TEXT("Attempting to load platform module: %s"), *ModuleName.ToString());
        IModuleInterface *Module = ModuleManager.LoadModule(ModuleName);
        if (Module == nullptr)
        {
            UE_LOG(LogRedpointEOSConfig, Error, TEXT("Failed to load platform module!"));
        }
        else
        {
            UE_LOG(LogRedpointEOSConfig, Verbose, TEXT("Loaded platform module has pointer value: %p"), Module);
        }
        if (bIsPrivatePlatform)
        {
            checkf(
                Module != nullptr,
                TEXT("Unable to load module '%s'; we could not load the native platform extension. For non-public "
                     "platforms, you must have a native platform extension so that the plugin can load the EOS SDK."),
                *ModuleName.ToString());
        }
        else
        {
            checkf(
                Module != nullptr,
                TEXT("Unable to load module '%s', which is a required module that should be built as part of the "
                     "Unreal Engine build process. If you experience this error, clear out 'Binaries' and "
                     "'Intermediate' and build your project again."),
                *ModuleName.ToString());
        }
    }
}

}