// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSPlatform/Services/RuntimePlatformAvatarService.h"
#include "RedpointEOSUserCache/UserInfo.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2821691268, Redpoint::EOS::Platform::Integration::OSSv1::Services)
{
using namespace ::Redpoint::EOS::Platform::Services;
using namespace ::Redpoint::EOS::UserCache;

class REDPOINTEOSPLATFORMINTEGRATIONOSSV1_API FOSSv1RuntimePlatformAvatarService
    : public IRuntimePlatformAvatarService,
      public TSharedFromThis<FOSSv1RuntimePlatformAvatarService>
{
private:
    EOS_EExternalAccountType ExternalAccountTypeToHandle;
    FName SubsystemName;

public:
    FOSSv1RuntimePlatformAvatarService(
        const EOS_EExternalAccountType &InExternalAccountTypeToHandle,
        const FName &InSubsystemName);
    UE_NONCOPYABLE(FOSSv1RuntimePlatformAvatarService);
    virtual ~FOSSv1RuntimePlatformAvatarService() override = default;

    virtual void GetAvatar(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const FExternalUserInfoRef &UserInfo,
        const FOnGetAvatarComplete &OnComplete) override;

    virtual void GetAvatarUrl(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const FExternalUserInfoRef &UserInfo,
        const FOnGetAvatarUrlComplete &OnComplete) override;
};

}; // namespace REDPOINT_EOS_FILE_NS_ID(2821691268,Redpoint::EOS::Platform::Integration::OSSv1::Services)

namespace Redpoint::EOS::Platform::Integration::OSSv1::Services
{
REDPOINT_EOS_FILE_NS_EXPORT(
    2821691268,
    Redpoint::EOS::Platform::Integration::OSSv1::Services,
    FOSSv1RuntimePlatformAvatarService)
}

REDPOINT_EOS_CODE_GUARD_END()