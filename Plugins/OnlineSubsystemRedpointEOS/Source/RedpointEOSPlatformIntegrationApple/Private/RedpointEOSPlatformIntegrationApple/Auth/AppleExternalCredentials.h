// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_APPLE_ENABLED
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSAuth/OnlineExternalCredentials.h"
#include "RedpointEOSPlatformIntegrationApple/Auth/AppleSignInControllerWrapper.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_APPLE_ENABLED

namespace REDPOINT_EOS_FILE_NS_ID(3580085967, Redpoint::EOS::Platform::Integration::Apple::Auth)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Auth;
using namespace ::Redpoint::EOS::Platform::Integration::Apple::Auth;

class FAppleExternalCredentials : public IOnlineExternalCredentials
{
private:
    FString Token;

public:
    FAppleExternalCredentials(const FString &InToken);
    UE_NONCOPYABLE(FAppleExternalCredentials);
    virtual ~FAppleExternalCredentials() override = default;

    virtual FText GetProviderDisplayName() const override;
    virtual FString GetType() const override;
    virtual FString GetId() const override;
    virtual FString GetToken() const override;
    virtual ESupportedLinkType GetProviderSupportedLinkType(FName PlatformName) const override;
    virtual TMap<FString, FString> GetAuthAttributes() const override;
    virtual FName GetNativeSubsystemName() const override;
    virtual void Refresh(
        TSoftObjectPtr<UWorld> InWorld,
        int32 LocalUserNum,
        FOnlineExternalCredentialsRefreshComplete OnComplete) override;

private:
    void OnSignInComplete(
        const FError& Error,
        const TOptional<FString>& AuthorizationCode,
        FOnlineExternalCredentialsRefreshComplete OnComplete);
};

}

namespace Redpoint::EOS::Platform::Integration::Apple::Auth
{
REDPOINT_EOS_FILE_NS_EXPORT(3580085967, Redpoint::EOS::Platform::Integration::Apple::Auth, FAppleExternalCredentials)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()
