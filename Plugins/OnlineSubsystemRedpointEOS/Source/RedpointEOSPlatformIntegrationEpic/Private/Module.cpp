// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSCore/Module.h"
#include "Modules/ModuleManager.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSPlatformIntegrationEpic/RuntimePlatformIntegrationEpic.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

class REDPOINTEOSPLATFORMINTEGRATIONEPIC_API FRedpointEOSPlatformIntegrationEpicModule : public FDefaultModuleImpl
{
public:
    virtual void StartupModule() override
    {
        using namespace ::Redpoint::EOS::Platform::Integration::Epic;
        using namespace ::Redpoint::EOS::Core;

        FModule::GetModuleChecked().GetRuntimePlatformRegistry()->AddRuntimePlatformIntegration(
            FName(TEXT("Epic")),
            MakeShared<FRuntimePlatformIntegrationEpic>());
    }

    virtual void ShutdownModule() override
    {
        using namespace ::Redpoint::EOS::Core;

        FModule::GetModuleChecked().GetRuntimePlatformRegistry()->RemoveRuntimePlatformIntegration(FName(TEXT("Epic")));
    }
};

REDPOINT_EOS_CODE_GUARD_END()

IMPLEMENT_MODULE(FRedpointEOSPlatformIntegrationEpicModule, RedpointEOSPlatformIntegrationEpic);