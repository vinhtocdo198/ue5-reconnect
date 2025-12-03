// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatformIntegrationGOG/RuntimePlatformIntegrationGOG.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && EOS_GOG_ENABLED

#include "RedpointEOSPlatformIntegrationGOG/Services/GOGRuntimePlatformLegacyService.h"

namespace Redpoint::EOS::Platform::Integration::GOG
{

FRuntimePlatformIntegrationGOG::FRuntimePlatformIntegrationGOG()
    : FRuntimePlatformIntegrationOSSv1(EOS_EExternalAccountType::EOS_EAT_GOG, FName(TEXT("GOG")))
    , LegacyService(MakeShared<Services::FGOGRuntimePlatformLegacyService>())
{
}

TSharedRef<Services::IRuntimePlatformLegacyService> FRuntimePlatformIntegrationGOG::GetLegacyService() const
{
    return this->LegacyService;
}

}

#endif