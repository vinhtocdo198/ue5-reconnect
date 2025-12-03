// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER && defined(REDPOINT_EOS_STEAM_ENABLED)
#include "RedpointEOSPlatform/RuntimePlatformIntegration.h"
#include "RedpointEOSPlatformIntegrationOSSv1/RuntimePlatformIntegrationOSSv1.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && defined(REDPOINT_EOS_STEAM_ENABLED)

namespace REDPOINT_EOS_FILE_NS_ID(1893130842, Redpoint::EOS::Platform::Integration::Steam)
{
using namespace ::Redpoint::EOS::Platform::Integration::OSSv1;
using namespace ::Redpoint::EOS::Platform::Services;

class FRuntimePlatformIntegrationSteam : public FRuntimePlatformIntegrationOSSv1
{
public:
    FRuntimePlatformIntegrationSteam();
    UE_NONCOPYABLE(FRuntimePlatformIntegrationSteam);
    virtual ~FRuntimePlatformIntegrationSteam() override = default;
};

}

namespace Redpoint::EOS::Platform::Integration::Steam
{
REDPOINT_EOS_FILE_NS_EXPORT(1893130842, Redpoint::EOS::Platform::Integration::Steam, FRuntimePlatformIntegrationSteam)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()