// Copyright June Rhodes. All Rights Reserved.

#include "Modules/ModuleManager.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_META_ENABLED
#include "RedpointEOSAuth/Graphs/AuthenticationGraphRuntimePlatform.h"
#include "RedpointEOSCore/Module.h"
#include "RedpointEOSPlatform/Android/AndroidEnvironment.h"
#include "RedpointEOSPlatformIntegrationMeta/PlatformName.h"
#include "RedpointEOSPlatformIntegrationMeta/RuntimePlatformIntegrationMeta.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1365703453, Redpoint::EOS::Platform::Integration::Meta)
{

class REDPOINTEOSPLATFORMINTEGRATIONMETA_API FRedpointEOSPlatformIntegrationMetaModule : public FDefaultModuleImpl
{
#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_META_ENABLED
public:
    virtual void StartupModule() override
    {
        using namespace ::Redpoint::EOS::Core;
#if PLATFORM_ANDROID
        using namespace ::Redpoint::EOS::Platform::Android;
#endif
        using namespace ::Redpoint::EOS::Platform::Integration::Meta;
        using namespace ::Redpoint::EOS::Auth::Graphs;

#if PLATFORM_ANDROID
        if (!FAndroidEnvironment::IsMetaQuest())
        {
            return;
        }
#endif

        FModule::GetModuleChecked().GetRuntimePlatformRegistry()->AddRuntimePlatformIntegration(
            PlatformNameMeta,
            MakeShared<FRuntimePlatformIntegrationMeta>());
    }

    virtual void ShutdownModule() override
    {
        using namespace ::Redpoint::EOS::Core;
#if PLATFORM_ANDROID
        using namespace ::Redpoint::EOS::Platform::Android;
#endif
        using namespace ::Redpoint::EOS::Platform::Integration::Meta;

#if PLATFORM_ANDROID
        if (!FAndroidEnvironment::IsMetaQuest())
        {
            return;
        }
#endif

        FModule::GetModuleChecked().GetRuntimePlatformRegistry()->RemoveRuntimePlatformIntegration(PlatformNameMeta);
    }
#endif
};

};

namespace Redpoint::EOS::Platform::Integration::Meta
{
REDPOINT_EOS_FILE_NS_EXPORT(
    1365703453,
    Redpoint::EOS::Platform::Integration::Meta,
    FRedpointEOSPlatformIntegrationMetaModule)
}

REDPOINT_EOS_CODE_GUARD_END()

IMPLEMENT_MODULE(
    ::Redpoint::EOS::Platform::Integration::Meta::FRedpointEOSPlatformIntegrationMetaModule,
    RedpointEOSPlatformIntegrationMeta);