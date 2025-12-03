// Copyright June Rhodes. All Rights Reserved.

#include "Modules/ModuleManager.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER && EOS_GOOGLE_PLAY_ENABLED
#include "RedpointEOSAuth/Graphs/AuthenticationGraphRuntimePlatform.h"
#include "RedpointEOSCore/Module.h"
#include "RedpointEOSPlatform/Android/AndroidEnvironment.h"
#include "RedpointEOSPlatformIntegrationGooglePlay/RuntimePlatformIntegrationGooglePlay.h"
#endif

class REDPOINTEOSPLATFORMINTEGRATIONGOOGLEPLAY_API FRedpointEOSPlatformIntegrationGooglePlayModule
    : public FDefaultModuleImpl
{
#if !REDPOINT_EOS_IS_DEDICATED_SERVER && EOS_GOOGLE_PLAY_ENABLED
public:
    virtual void StartupModule() override
    {
        using namespace ::Redpoint::EOS::Core;
        using namespace ::Redpoint::EOS::Platform::Android;
        using namespace ::Redpoint::EOS::Platform::Integration::GooglePlay;
        using namespace ::Redpoint::EOS::Auth::Graphs;

        if (FAndroidEnvironment::IsMetaQuest())
        {
            return;
        }

        FModule::GetModuleChecked().GetRuntimePlatformRegistry()->AddRuntimePlatformIntegration(
            FName(TEXT("Google")),
            MakeShared<FRuntimePlatformIntegrationGooglePlay>());
    }

    virtual void ShutdownModule() override
    {
        using namespace ::Redpoint::EOS::Core;
        using namespace ::Redpoint::EOS::Platform::Android;

        if (FAndroidEnvironment::IsMetaQuest())
        {
            return;
        }

        FModule::GetModuleChecked().GetRuntimePlatformRegistry()->RemoveRuntimePlatformIntegration(
            FName(TEXT("Google")));
    }
#endif
};

IMPLEMENT_MODULE(FRedpointEOSPlatformIntegrationGooglePlayModule, RedpointEOSPlatformIntegrationGooglePlay);