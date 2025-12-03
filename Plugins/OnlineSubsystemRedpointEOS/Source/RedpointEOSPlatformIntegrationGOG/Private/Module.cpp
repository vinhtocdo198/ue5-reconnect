// Copyright June Rhodes. All Rights Reserved.

#include "Modules/ModuleManager.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER && EOS_GOG_ENABLED
#include "OnlineSubsystem.h"
#include "RedpointEOSAuth/Graphs/AuthenticationGraphOnlineSubsystem.h"
#include "RedpointEOSCore/Module.h"
#include "RedpointEOSPlatformIntegrationGOG/GetExternalCredentialsFromGOGNode.h"
#include "RedpointEOSPlatformIntegrationGOG/RuntimePlatformIntegrationGOG.h"
#endif

class REDPOINTEOSPLATFORMINTEGRATIONGOG_API FRedpointEOSPlatformIntegrationGOGModule : public FDefaultModuleImpl
{
#if !REDPOINT_EOS_IS_DEDICATED_SERVER && EOS_GOG_ENABLED
public:
    virtual void StartupModule() override
    {
        using namespace ::Redpoint::EOS::Core;
        using namespace ::Redpoint::EOS::Platform::Integration::GOG;
        using namespace ::Redpoint::EOS::Auth::Graphs;

        FModule::GetModuleChecked().GetRuntimePlatformRegistry()->AddRuntimePlatformIntegration(
            FName(TEXT("GOG")),
            MakeShared<FRuntimePlatformIntegrationGOG>());

        FAuthenticationGraphOnlineSubsystem::RegisterForCustomPlatform(
            FName(TEXT("GOG")),
            NSLOCTEXT("OnlineSubsystemRedpointEOS", "AuthGraph_GOG", "GOG Galaxy Only"),
            FName(TEXT("GOG")),
            EOS_EExternalCredentialType::EOS_ECT_GOG_SESSION_TICKET,
            TEXT("gog"),
            TEXT("gog.encryptedAppTicket"),
            MakeShared<FGetExternalCredentialsFromGOGNode>());
    }

    virtual void ShutdownModule() override
    {
        using namespace ::Redpoint::EOS::Core;

        FModule::GetModuleChecked().GetRuntimePlatformRegistry()->RemoveRuntimePlatformIntegration(FName(TEXT("GOG")));
    }
#endif
};

IMPLEMENT_MODULE(FRedpointEOSPlatformIntegrationGOGModule, RedpointEOSPlatformIntegrationGOG);