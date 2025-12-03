// Copyright June Rhodes. All Rights Reserved.

#pragma once

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && EOS_GOG_ENABLED

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSPlatform/RuntimePlatformIntegration.h"
#include "RedpointEOSPlatformIntegrationOSSv1/RuntimePlatformIntegrationOSSv1.h"

namespace Redpoint::EOS::Platform::Integration::GOG
{
using namespace ::Redpoint::EOS::Platform::Services;
using namespace ::Redpoint::EOS::Platform::Integration::OSSv1;

class FRuntimePlatformIntegrationGOG : public FRuntimePlatformIntegrationOSSv1
{
private:
    TSharedRef<IRuntimePlatformLegacyService> LegacyService;

public:
    FRuntimePlatformIntegrationGOG();
    UE_NONCOPYABLE(FRuntimePlatformIntegrationGOG);
    virtual ~FRuntimePlatformIntegrationGOG() = default;

    virtual TSharedRef<IRuntimePlatformLegacyService> GetLegacyService() const override;
};

}

#endif