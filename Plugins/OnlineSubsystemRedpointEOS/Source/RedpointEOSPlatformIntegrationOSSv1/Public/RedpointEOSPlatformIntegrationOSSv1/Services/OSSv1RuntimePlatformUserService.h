// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSPlatform/RuntimePlatformIntegration.h"
#include "RedpointEOSPlatform/Services/RuntimePlatformUserService.h"
#include "RedpointEOSPlatformIntegrationOSSv1/ResolveUserIdToExternalAccountIdInfo.h"
#include "RedpointEOSUserCache/UserInfo.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(924768428, Redpoint::EOS::Platform::Integration::OSSv1::Services)
{
using namespace ::Redpoint::EOS::Platform;
using namespace ::Redpoint::EOS::Platform::Services;
using namespace ::Redpoint::EOS::Platform::Integration::OSSv1;

class REDPOINTEOSPLATFORMINTEGRATIONOSSV1_API FOSSv1RuntimePlatformUserService
    : public IRuntimePlatformUserService,
      public TSharedFromThis<FOSSv1RuntimePlatformUserService>
{
private:
    EOS_EExternalAccountType ExternalAccountTypeToHandle;
    FName SubsystemName;
    FResolveUserIdToExternalAccountIdInfo ExternalInfoResolver;

public:
    FOSSv1RuntimePlatformUserService(
        const EOS_EExternalAccountType &InExternalAccountTypeToHandle,
        const FName &InSubsystemName,
        const FResolveUserIdToExternalAccountIdInfo &InExternalInfoResolver);
    UE_NONCOPYABLE(FOSSv1RuntimePlatformUserService);
    virtual ~FOSSv1RuntimePlatformUserService() override = default;

    virtual void GetExternalUserByUniqueNetId(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const FUniqueNetId &TargetUserId,
        const FOnGetExternalUserByUniqueNetIdComplete &OnComplete) override;

    virtual void GetExternalUserByAccountId(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        UE::Online::FAccountId TargetUserId,
        const FOnGetExternalUserByAccountIdComplete &OnComplete) override;
};

}; // namespace REDPOINT_EOS_FILE_NS_ID(924768428,Redpoint::EOS::Platform::Integration::OSSv1::Services)

namespace Redpoint::EOS::Platform::Integration::OSSv1::Services
{
REDPOINT_EOS_FILE_NS_EXPORT(
    924768428,
    Redpoint::EOS::Platform::Integration::OSSv1::Services,
    FOSSv1RuntimePlatformUserService)
}

REDPOINT_EOS_CODE_GUARD_END()