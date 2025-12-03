// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSPlatform/Services/RuntimePlatformUserService.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(93821324, Redpoint::EOS::Platform::Services)
{
using namespace ::Redpoint::EOS::UserCache;
using namespace ::Redpoint::EOS::Platform::Services;

class FNullRuntimePlatformUserService : public IRuntimePlatformUserService
{
public:
    FNullRuntimePlatformUserService() = default;
    UE_NONCOPYABLE(FNullRuntimePlatformUserService);
    virtual ~FNullRuntimePlatformUserService() override = default;

    virtual void GetExternalUserByUniqueNetId(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const FUniqueNetId &TargetUserId,
        const FOnGetExternalUserByUniqueNetIdComplete &OnComplete) override;

    virtual void GetExternalUserByAccountId(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        UE::Online::FAccountId TargetUserId,
        const FOnGetExternalUserByAccountIdComplete &OnComplete) override;
};

extern TSharedRef<IRuntimePlatformUserService> NullUserService;

}

namespace Redpoint::EOS::Platform::Services
{
REDPOINT_EOS_FILE_NS_EXPORT(93821324, Redpoint::EOS::Platform::Services, FNullRuntimePlatformUserService)
REDPOINT_EOS_FILE_NS_EXPORT(93821324, Redpoint::EOS::Platform::Services, NullUserService)
}

REDPOINT_EOS_CODE_GUARD_END()