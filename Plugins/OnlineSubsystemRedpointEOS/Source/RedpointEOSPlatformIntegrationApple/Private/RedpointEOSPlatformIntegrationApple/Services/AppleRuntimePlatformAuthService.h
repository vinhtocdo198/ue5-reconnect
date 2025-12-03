// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_APPLE_ENABLED
#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSPlatform/Services/RuntimePlatformAuthService.h"
#include "RedpointEOSPlatformIntegrationApple/Auth/AppleExternalCredentials.h"
#include "RedpointEOSPlatformIntegrationApple/Auth/AppleSignInControllerWrapper.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_APPLE_ENABLED

namespace REDPOINT_EOS_FILE_NS_ID(3138795473, Redpoint::EOS::Platform::Integration::Apple::Services)
{
using namespace ::Redpoint::EOS::Auth;
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Platform::Integration::Apple::Auth;
using namespace ::Redpoint::EOS::Platform::Services;

class FAppleRuntimePlatformAuthService : public IRuntimePlatformAuthService, public TSharedFromThis<FAppleRuntimePlatformAuthService>
{
private:
    TSharedPtr<FAppleExternalCredentials> SignedInUser;

public:
    FAppleRuntimePlatformAuthService() = default;
    UE_NONCOPYABLE(FAppleRuntimePlatformAuthService);
    virtual ~FAppleRuntimePlatformAuthService() override = default;

    virtual FName GetPlatformName() const override;

    virtual int32 GetSelectionScore(const FRuntimePlatformServiceCallContextRef &CallContext) const override;

    virtual void EnsureSignedInOnLocalPlatform(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const FOnEnsureSignedInOnLocalPlatform &OnComplete) override;

private:
    void OnSignInComplete(
        const FError& Error,
        const TOptional<FString>& AuthorizationCode,
        FRuntimePlatformServiceCallContextRef CallContext,
        FOnEnsureSignedInOnLocalPlatform OnComplete);

public:
    virtual void IsOnlinePlayPermitted(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const FOnIsOnlinePlayPermitted &OnComplete) override;

    virtual void GetCredentialsFromLocalPlatformDuringAuthenticationGraph(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const TSharedRef<FAuthenticationGraphState> &State,
        const FOnGetCredentialsFromLocalPlatformDuringAuthenticationGraph &OnComplete) override;
};

};

namespace Redpoint::EOS::Platform::Integration::Apple::Services
{
REDPOINT_EOS_FILE_NS_EXPORT(
    3138795473,
    Redpoint::EOS::Platform::Integration::Apple::Services,
    FAppleRuntimePlatformAuthService)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()
