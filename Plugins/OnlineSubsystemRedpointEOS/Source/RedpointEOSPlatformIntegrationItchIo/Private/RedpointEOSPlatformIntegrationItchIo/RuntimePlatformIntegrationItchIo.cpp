// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatformIntegrationItchIo/RuntimePlatformIntegrationItchIo.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_ITCH_IO_ENABLED
#include "RedpointEOSPlatformIntegrationItchIo/Services/ItchIoRuntimePlatformAuthService.h"
#include "RedpointEOSPlatformIntegrationItchIo/Services/ItchIoRuntimePlatformAvatarService.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_ITCH_IO_ENABLED

namespace REDPOINT_EOS_FILE_NS_ID(2565810172, Redpoint::EOS::Platform::Integration::ItchIo)
{
using namespace ::Redpoint::EOS::Platform::Integration::ItchIo::Services;

FRuntimePlatformIntegrationItchIo::FRuntimePlatformIntegrationItchIo()
    : AvatarService(MakeShared<FItchIoRuntimePlatformAvatarService>())
    , AuthService(MakeShared<FItchIoRuntimePlatformAuthService>())
{
}

TSharedRef<IRuntimePlatformAvatarService> FRuntimePlatformIntegrationItchIo::GetAvatarService() const
{
    return this->AvatarService;
}

TSharedRef<IRuntimePlatformAuthService> FRuntimePlatformIntegrationItchIo::GetAuthService() const
{
    return this->AuthService;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()