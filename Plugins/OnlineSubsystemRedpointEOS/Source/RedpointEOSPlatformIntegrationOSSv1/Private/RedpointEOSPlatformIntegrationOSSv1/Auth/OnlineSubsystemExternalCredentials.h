// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSAuth/OnlineExternalCredentials.h"
#include "RedpointEOSAuth/OnlineProvidedCredentials.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(270551599, Redpoint::EOS::Platform::Integration::OSSv1::Auth)
{
using namespace ::Redpoint::EOS::Auth;

class FOnlineSubsystemExternalCredentials : public IOnlineExternalCredentials
{
private:
    FName SubsystemName;
    EOS_EExternalCredentialType CredentialType;
    FOnlineProvidedCredentials CurrentCredentials;
    FString AuthenticatedWithValue;
    FString TokenAuthAttributeName;

public:
    UE_NONCOPYABLE(FOnlineSubsystemExternalCredentials);
    FOnlineSubsystemExternalCredentials(
        FName InSubsystemName,
        EOS_EExternalCredentialType InCredentialType,
        const FOnlineProvidedCredentials &InitialCredentials,
        const FString &InAuthenticatedWithValue,
        const FString &InTokenAuthAttributeName);
    virtual ~FOnlineSubsystemExternalCredentials() override = default;
    virtual FText GetProviderDisplayName() const override;
    virtual FString GetType() const override;
    virtual FString GetId() const override;
    virtual FString GetToken() const override;
    virtual TMap<FString, FString> GetAuthAttributes() const override;
    virtual FName GetNativeSubsystemName() const override;
    virtual void Refresh(
        TSoftObjectPtr<UWorld> InWorld,
        int32 LocalUserNum,
        FOnlineExternalCredentialsRefreshComplete OnComplete) override;
};

}

namespace Redpoint::EOS::Platform::Integration::OSSv1::Auth
{
REDPOINT_EOS_FILE_NS_EXPORT(
    270551599,
    Redpoint::EOS::Platform::Integration::OSSv1::Auth,
    FOnlineSubsystemExternalCredentials)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()