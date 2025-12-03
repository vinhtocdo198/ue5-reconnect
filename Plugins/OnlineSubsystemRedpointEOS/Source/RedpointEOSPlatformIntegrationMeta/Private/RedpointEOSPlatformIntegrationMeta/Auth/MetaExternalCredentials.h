// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_META_ENABLED
#include "OVR_Platform.h"
#include "OVR_User.h"
#include "RedpointEOSAuth/OnlineExternalCredentials.h"
#include "UObject/SoftObjectPtr.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_META_ENABLED

namespace REDPOINT_EOS_FILE_NS_ID(3113027389, Redpoint::EOS::Platform::Integration::Meta::Auth)
{
using namespace ::Redpoint::EOS::Auth;

class FMetaExternalCredentials : public IOnlineExternalCredentials
{
private:
    FString ProofToken;
    FString DisplayName;
    TMap<FString, FString> AuthAttributes;
    FString UserId;

public:
    FMetaExternalCredentials(
        const FString &InProofToken,
        const FString &InDisplayName,
        const TMap<FString, FString> &InAuthAttributes,
        const FString &InUserId);
    UE_NONCOPYABLE(FMetaExternalCredentials);
    virtual ~FMetaExternalCredentials() override = default;

    virtual FText GetProviderDisplayName() const override;
    virtual FString GetType() const override;
    virtual FString GetId() const override;
    virtual FString GetToken() const override;
    virtual ESupportedLinkType GetProviderSupportedLinkType(FName PlatformName) const override;
    virtual void StoreData(const FString &Key, const FString &Data) const override;
    virtual TOptional<FString> ReadData(const FString &key) const override;
    virtual TMap<FString, FString> GetAuthAttributes() const override;
    virtual FName GetNativeSubsystemName() const override;
    virtual void Refresh(
        TSoftObjectPtr<UWorld> InWorld,
        int32 LocalUserNum,
        FOnlineExternalCredentialsRefreshComplete OnComplete) override;
};

}

namespace Redpoint::EOS::Platform::Integration::Meta::Auth
{
REDPOINT_EOS_FILE_NS_EXPORT(3113027389, Redpoint::EOS::Platform::Integration::Meta::Auth, FMetaExternalCredentials)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()