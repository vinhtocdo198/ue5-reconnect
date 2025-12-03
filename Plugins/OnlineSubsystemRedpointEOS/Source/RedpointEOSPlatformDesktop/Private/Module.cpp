// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSCore/Module.h"
#include "Modules/ModuleManager.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSPlatformDesktop/RuntimePlatformLinux.h"
#include "RedpointEOSPlatformDesktop/RuntimePlatformMac.h"
#include "RedpointEOSPlatformDesktop/RuntimePlatformWindows.h"

class REDPOINTEOSPLATFORMDESKTOP_API FRedpointEOSPlatformDesktopModule : public FDefaultModuleImpl
{
public:
    virtual void StartupModule() override
    {
        using namespace ::Redpoint::EOS::Core;
        using namespace ::Redpoint::EOS::Platform::Desktop;

        FModule::GetModuleChecked()
#if PLATFORM_WINDOWS
            .SetRuntimePlatform(MakeShared<FRuntimePlatformWindows>())
#elif PLATFORM_MAC
            .SetRuntimePlatform(MakeShared<FRuntimePlatformMac>())
#elif PLATFORM_LINUX
            .SetRuntimePlatform(MakeShared<FRuntimePlatformLinux>())
#endif
            ;

        // Optionally load all the relevant desktop modules; these will register authentication graphs and runtime
        // integrations if they are available.
        FModuleManager::Get().LoadModule(
            FName(TEXT("RedpointEOSPlatformIntegrationDiscord")),
            ELoadModuleFlags::LogFailures);
        FModuleManager::Get().LoadModule(
            FName(TEXT("RedpointEOSPlatformIntegrationEpic")),
            ELoadModuleFlags::LogFailures);
        FModuleManager::Get().LoadModule(
            FName(TEXT("RedpointEOSPlatformIntegrationGOG")),
            ELoadModuleFlags::LogFailures);
        FModuleManager::Get().LoadModule(
            FName(TEXT("RedpointEOSPlatformIntegrationItchIo")),
            ELoadModuleFlags::LogFailures);
        FModuleManager::Get().LoadModule(
            FName(TEXT("RedpointEOSPlatformIntegrationMeta")),
            ELoadModuleFlags::LogFailures);
        FModuleManager::Get().LoadModule(
            FName(TEXT("RedpointEOSPlatformIntegrationSteam")),
            ELoadModuleFlags::LogFailures);
    }
};

IMPLEMENT_MODULE(FRedpointEOSPlatformDesktopModule, RedpointEOSPlatformDesktop);