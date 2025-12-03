// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_ITCH_IO_ENABLED
#include "RedpointEOSPlatform/Services/RuntimePlatformAvatarService.h"
#include "RedpointEOSUserCache/ExternalUserInfo.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_ITCH_IO_ENABLED

namespace REDPOINT_EOS_FILE_NS_ID(280458675, Redpoint::EOS::Platform::Integration::ItchIo::Services)
{
using namespace ::Redpoint::EOS::Platform::Services;
using namespace ::Redpoint::EOS::UserCache;

class FItchIoRuntimePlatformAvatarService : public IRuntimePlatformAvatarService,
                                            public TSharedFromThis<FItchIoRuntimePlatformAvatarService>
{
public:
    FItchIoRuntimePlatformAvatarService() = default;
    UE_NONCOPYABLE(FItchIoRuntimePlatformAvatarService);
    virtual ~FItchIoRuntimePlatformAvatarService() override = default;

    virtual void GetAvatar(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const FExternalUserInfoRef &UserInfo,
        const FOnGetAvatarComplete &OnComplete) override;
    virtual void GetAvatarUrl(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const FExternalUserInfoRef &UserInfo,
        const FOnGetAvatarUrlComplete &OnComplete) override;
};

}

namespace Redpoint::EOS::Platform::Integration::ItchIo::Services
{
REDPOINT_EOS_FILE_NS_EXPORT(
    280458675,
    Redpoint::EOS::Platform::Integration::ItchIo::Services,
    FItchIoRuntimePlatformAvatarService)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()