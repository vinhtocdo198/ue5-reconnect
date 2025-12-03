// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_APPLE_ENABLED
#include "RedpointEOSPlatform/RuntimePlatformIntegration.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_APPLE_ENABLED

namespace REDPOINT_EOS_FILE_NS_ID(1682586091, Redpoint::EOS::Platform::Integration::Apple)
{
using namespace ::Redpoint::EOS::Platform;
using namespace ::Redpoint::EOS::Platform::Services;

class FRuntimePlatformIntegrationApple : public IRuntimePlatformIntegration
{
private:
    TSharedRef<IRuntimePlatformAuthService> AuthService;

public:
    FRuntimePlatformIntegrationApple();
    UE_NONCOPYABLE(FRuntimePlatformIntegrationApple);
    virtual ~FRuntimePlatformIntegrationApple() override = default;

    virtual TSharedRef<IRuntimePlatformAuthService> GetAuthService() const override;
};

}

namespace Redpoint::EOS::Platform::Integration::Apple
{
REDPOINT_EOS_FILE_NS_EXPORT(1682586091, Redpoint::EOS::Platform::Integration::Apple, FRuntimePlatformIntegrationApple)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()