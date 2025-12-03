// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatformIntegrationApple/RuntimePlatformIntegrationApple.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_APPLE_ENABLED
#include "RedpointEOSPlatformIntegrationApple/Services/AppleRuntimePlatformAuthService.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_APPLE_ENABLED

namespace REDPOINT_EOS_FILE_NS_ID(1682586091, Redpoint::EOS::Platform::Integration::Apple)
{
using namespace ::Redpoint::EOS::Platform::Integration::Apple::Services;
using namespace ::Redpoint::EOS::Platform::Services;
using namespace ::Redpoint::EOS::Platform::Types;

FRuntimePlatformIntegrationApple::FRuntimePlatformIntegrationApple()
    : AuthService(MakeShared<FAppleRuntimePlatformAuthService>())
{
}

TSharedRef<IRuntimePlatformAuthService> FRuntimePlatformIntegrationApple::GetAuthService() const
{
    return this->AuthService;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()