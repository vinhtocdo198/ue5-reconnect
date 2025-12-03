// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER && EOS_GOOGLE_PLAY_ENABLED
#include "RedpointEOSPlatformIntegrationOSSv1/Services/OSSv1RuntimePlatformAuthService.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && EOS_GOOGLE_PLAY_ENABLED

namespace REDPOINT_EOS_FILE_NS_ID(2109748584, Redpoint::EOS::Platform::Integration::GooglePlay::Services)
{
using namespace ::Redpoint::EOS::Platform::Integration::OSSv1::Services;
using namespace ::Redpoint::EOS::Platform::Services;

class FGooglePlayRuntimePlatformAuthService : public FOSSv1RuntimePlatformAuthService
{
public:
    FGooglePlayRuntimePlatformAuthService(
        FName InPlatformName,
        EOS_EExternalCredentialType InObtainedCredentialType,
        const FString &InAuthenticatedWithValue,
        const FString &InTokenAuthAttributeName);
    UE_NONCOPYABLE(FGooglePlayRuntimePlatformAuthService);
    virtual ~FGooglePlayRuntimePlatformAuthService() override = default;

    virtual int32 GetSelectionScore(const FRuntimePlatformServiceCallContextRef &CallContext) const override;
};

};

namespace Redpoint::EOS::Platform::Integration::GooglePlay::Services
{
REDPOINT_EOS_FILE_NS_EXPORT(
    2109748584,
    Redpoint::EOS::Platform::Integration::GooglePlay::Services,
    FGooglePlayRuntimePlatformAuthService)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()