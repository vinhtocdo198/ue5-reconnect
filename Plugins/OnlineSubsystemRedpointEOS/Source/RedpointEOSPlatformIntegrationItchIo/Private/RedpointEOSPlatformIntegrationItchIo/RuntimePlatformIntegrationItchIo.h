// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_ITCH_IO_ENABLED
#include "RedpointEOSPlatform/RuntimePlatformIntegration.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_ITCH_IO_ENABLED

namespace REDPOINT_EOS_FILE_NS_ID(2565810172, Redpoint::EOS::Platform::Integration::ItchIo)
{
using namespace ::Redpoint::EOS::Platform;
using namespace ::Redpoint::EOS::Platform::Services;

class FRuntimePlatformIntegrationItchIo : public IRuntimePlatformIntegration
{
private:
    TSharedRef<IRuntimePlatformAvatarService> AvatarService;
    TSharedRef<IRuntimePlatformAuthService> AuthService;

public:
    FRuntimePlatformIntegrationItchIo();
    UE_NONCOPYABLE(FRuntimePlatformIntegrationItchIo);
    virtual ~FRuntimePlatformIntegrationItchIo() override = default;

    virtual TSharedRef<IRuntimePlatformAvatarService> GetAvatarService() const override;
    virtual TSharedRef<IRuntimePlatformAuthService> GetAuthService() const override;
};

}

namespace Redpoint::EOS::Platform::Integration::ItchIo
{
REDPOINT_EOS_FILE_NS_EXPORT(2565810172, Redpoint::EOS::Platform::Integration::ItchIo, FRuntimePlatformIntegrationItchIo)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()