// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_META_ENABLED
#include "RedpointEOSPlatform/Services/RuntimePlatformAvatarService.h"
#include "RedpointEOSUserCache/ExternalUserInfo.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_META_ENABLED

namespace REDPOINT_EOS_FILE_NS_ID(3278636343, Redpoint::EOS::Platform::Integration::Meta::Services)
{
using namespace ::Redpoint::EOS::Platform::Services;
using namespace ::Redpoint::EOS::UserCache;

class FMetaRuntimePlatformAvatarService : public IRuntimePlatformAvatarService,
                                          public TSharedFromThis<FMetaRuntimePlatformAvatarService>
{
public:
    FMetaRuntimePlatformAvatarService() = default;
    UE_NONCOPYABLE(FMetaRuntimePlatformAvatarService);
    virtual ~FMetaRuntimePlatformAvatarService() override = default;

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

namespace Redpoint::EOS::Platform::Integration::Meta::Services
{
REDPOINT_EOS_FILE_NS_EXPORT(
    3278636343,
    Redpoint::EOS::Platform::Integration::Meta::Services,
    FMetaRuntimePlatformAvatarService)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()