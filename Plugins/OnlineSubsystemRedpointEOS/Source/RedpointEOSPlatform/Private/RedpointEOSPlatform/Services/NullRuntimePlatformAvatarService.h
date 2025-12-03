// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSPlatform/Services/RuntimePlatformAvatarService.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(10947481, Redpoint::EOS::Platform::Services)
{
using namespace ::Redpoint::EOS::UserCache;
using namespace ::Redpoint::EOS::Platform::Services;

class FNullRuntimePlatformAvatarService : public IRuntimePlatformAvatarService
{
public:
    FNullRuntimePlatformAvatarService() = default;
    UE_NONCOPYABLE(FNullRuntimePlatformAvatarService);
    virtual ~FNullRuntimePlatformAvatarService() override = default;

    virtual void GetAvatar(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const FExternalUserInfoRef &UserInfo,
        const IRuntimePlatformAvatarService::FOnGetAvatarComplete &OnComplete) override;

    virtual void GetAvatarUrl(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const FExternalUserInfoRef &UserInfo,
        const IRuntimePlatformAvatarService::FOnGetAvatarUrlComplete &OnComplete) override;
};

extern TSharedRef<IRuntimePlatformAvatarService> NullAvatarService;

}

namespace Redpoint::EOS::Platform::Services
{
REDPOINT_EOS_FILE_NS_EXPORT(10947481, Redpoint::EOS::Platform::Services, FNullRuntimePlatformAvatarService)
REDPOINT_EOS_FILE_NS_EXPORT(10947481, Redpoint::EOS::Platform::Services, NullAvatarService)
}

REDPOINT_EOS_CODE_GUARD_END()