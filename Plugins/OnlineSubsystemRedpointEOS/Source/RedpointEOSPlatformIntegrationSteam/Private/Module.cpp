// Copyright June Rhodes. All Rights Reserved.

#include "Modules/ModuleManager.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER && EOS_STEAM_ENABLED
#include "RedpointEOSAuth/Graphs/AuthenticationGraphRuntimePlatform.h"
#include "RedpointEOSCore/Module.h"
#include "RedpointEOSPlatformIntegrationSteam/RuntimePlatformIntegrationSteam.h"
#endif

class REDPOINTEOSPLATFORMINTEGRATIONSTEAM_API FRedpointEOSPlatformIntegrationSteamModule : public FDefaultModuleImpl
{
#if !REDPOINT_EOS_IS_DEDICATED_SERVER && EOS_STEAM_ENABLED
public:
    virtual void StartupModule() override
    {
        using namespace ::Redpoint::EOS::Core;
        using namespace ::Redpoint::EOS::Platform::Integration::Steam;
        using namespace ::Redpoint::EOS::Auth::Graphs;

        FModule::GetModuleChecked().GetRuntimePlatformRegistry()->AddRuntimePlatformIntegration(
            FName(TEXT("Steam")),
            MakeShared<FRuntimePlatformIntegrationSteam>());
    }

    virtual void ShutdownModule() override
    {
        using namespace ::Redpoint::EOS::Core;

        FModule::GetModuleChecked().GetRuntimePlatformRegistry()->RemoveRuntimePlatformIntegration(
            FName(TEXT("Steam")));
    }
#endif
};

IMPLEMENT_MODULE(FRedpointEOSPlatformIntegrationSteamModule, RedpointEOSPlatformIntegrationSteam);