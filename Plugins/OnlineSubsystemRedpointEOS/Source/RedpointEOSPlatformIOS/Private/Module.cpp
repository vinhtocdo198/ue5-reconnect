// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSCore/Module.h"
#include "Modules/ModuleManager.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSPlatformIOS/RuntimePlatformIOS.h"

class REDPOINTEOSPLATFORMIOS_API FRedpointEOSPlatformIOSModule : public FDefaultModuleImpl
{
public:
    virtual void StartupModule() override
    {
        using namespace ::Redpoint::EOS::Core;
        using namespace ::Redpoint::EOS::Platform::IOS;

        FModule::GetModuleChecked().SetRuntimePlatform(MakeShared<FRuntimePlatformIOS>());

        // Optionally load the Sign-in-with-Apple integration.
        FModuleManager::Get().LoadModule(
            FName(TEXT("RedpointEOSPlatformIntegrationApple")),
            ELoadModuleFlags::LogFailures);
    }
};

IMPLEMENT_MODULE(FRedpointEOSPlatformIOSModule, RedpointEOSPlatformIOS);