// Copyright June Rhodes. All Rights Reserved.

#include "Modules/ModuleManager.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_ITCH_IO_ENABLED
#include "RedpointEOSAuth/Graphs/AuthenticationGraphRuntimePlatform.h"
#include "RedpointEOSCore/Module.h"
#include "RedpointEOSPlatformIntegrationItchIo/PlatformName.h"
#include "RedpointEOSPlatformIntegrationItchIo/RuntimePlatformIntegrationItchIo.h"
#endif

class REDPOINTEOSPLATFORMINTEGRATIONITCHIO_API FRedpointEOSPlatformIntegrationItchIoModule : public FDefaultModuleImpl
{
#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_ITCH_IO_ENABLED
public:
    virtual void StartupModule() override
    {
        using namespace ::Redpoint::EOS::Core;
        using namespace ::Redpoint::EOS::Platform::Integration::ItchIo;
        using namespace ::Redpoint::EOS::Auth::Graphs;

        FModule::GetModuleChecked().GetRuntimePlatformRegistry()->AddRuntimePlatformIntegration(
            PlatformNameItchIo,
            MakeShared<FRuntimePlatformIntegrationItchIo>());
    }

    virtual void ShutdownModule() override
    {
        using namespace ::Redpoint::EOS::Core;
        using namespace ::Redpoint::EOS::Platform::Integration::ItchIo;

        FModule::GetModuleChecked().GetRuntimePlatformRegistry()->RemoveRuntimePlatformIntegration(PlatformNameItchIo);
    }
#endif
};

IMPLEMENT_MODULE(FRedpointEOSPlatformIntegrationItchIoModule, RedpointEOSPlatformIntegrationItchIo);