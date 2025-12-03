// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSCore/Module.h"
#include "Modules/ModuleManager.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSPlatformAndroid/RuntimePlatformAndroid.h"

class REDPOINTEOSPLATFORMANDROID_API FRedpointEOSPlatformAndroidModule : public FDefaultModuleImpl
{
public:
    virtual void StartupModule() override
    {
        using namespace ::Redpoint::EOS::Core;
        using namespace ::Redpoint::EOS::Platform::Android;

        FModule::GetModuleChecked().SetRuntimePlatform(MakeShared<FRuntimePlatformAndroid>());

        // Optionally load the Google Play integration for when we're running on mobile devices.
        FModuleManager::Get().LoadModule(
            FName(TEXT("RedpointEOSPlatformIntegrationGooglePlay")),
            ELoadModuleFlags::LogFailures);

        // Optionally load the Meta integration for when we're running on Quest.
        FModuleManager::Get().LoadModule(
            FName(TEXT("RedpointEOSPlatformIntegrationMeta")),
            ELoadModuleFlags::LogFailures);
    }
};

IMPLEMENT_MODULE(FRedpointEOSPlatformAndroidModule, RedpointEOSPlatformAndroid);