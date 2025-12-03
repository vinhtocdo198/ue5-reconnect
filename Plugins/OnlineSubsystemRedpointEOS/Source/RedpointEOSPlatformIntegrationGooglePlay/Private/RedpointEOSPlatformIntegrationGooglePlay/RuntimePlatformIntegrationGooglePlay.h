// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER && EOS_GOOGLE_PLAY_ENABLED
#include "RedpointEOSPlatform/RuntimePlatformIntegration.h"
#include "RedpointEOSPlatformIntegrationOSSv1/RuntimePlatformIntegrationOSSv1.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && EOS_GOOGLE_PLAY_ENABLED

namespace REDPOINT_EOS_FILE_NS_ID(4079654914, Redpoint::EOS::Platform::Integration::GooglePlay)
{
using namespace ::Redpoint::EOS::Platform::Services;
using namespace ::Redpoint::EOS::Platform::Integration::OSSv1;

class FRuntimePlatformIntegrationGooglePlay : public FRuntimePlatformIntegrationOSSv1
{
public:
    FRuntimePlatformIntegrationGooglePlay();
    UE_NONCOPYABLE(FRuntimePlatformIntegrationGooglePlay);
    virtual ~FRuntimePlatformIntegrationGooglePlay() override = default;
};

}

namespace Redpoint::EOS::Platform::Integration::GooglePlay
{
REDPOINT_EOS_FILE_NS_EXPORT(
    4079654914,
    Redpoint::EOS::Platform::Integration::GooglePlay,
    FRuntimePlatformIntegrationGooglePlay)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()