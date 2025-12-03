// Copyright June Rhodes. All Rights Reserved.

#include "Modules/ModuleManager.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_APPLE_ENABLED
#include "RedpointEOSAuth/Graphs/AuthenticationGraphRuntimePlatform.h"
#include "RedpointEOSCore/Module.h"
#include "RedpointEOSPlatformIntegrationApple/RuntimePlatformIntegrationApple.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

class REDPOINTEOSPLATFORMINTEGRATIONAPPLE_API FRedpointEOSPlatformIntegrationAppleModule : public FDefaultModuleImpl
{
#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_APPLE_ENABLED
public:
    virtual void StartupModule() override
    {
        using namespace ::Redpoint::EOS::Core;
        using namespace ::Redpoint::EOS::Platform::Integration::Apple;
        using namespace ::Redpoint::EOS::Auth::Graphs;

        FModule::GetModuleChecked().GetRuntimePlatformRegistry()->AddRuntimePlatformIntegration(
            FName(TEXT("Apple")),
            MakeShared<FRuntimePlatformIntegrationApple>());
    }

    virtual void ShutdownModule() override
    {
        using namespace ::Redpoint::EOS::Core;

        FModule::GetModuleChecked().GetRuntimePlatformRegistry()->RemoveRuntimePlatformIntegration(
            FName(TEXT("Apple")));
    }
#endif
};

REDPOINT_EOS_CODE_GUARD_END()

IMPLEMENT_MODULE(FRedpointEOSPlatformIntegrationAppleModule, RedpointEOSPlatformIntegrationApple);
