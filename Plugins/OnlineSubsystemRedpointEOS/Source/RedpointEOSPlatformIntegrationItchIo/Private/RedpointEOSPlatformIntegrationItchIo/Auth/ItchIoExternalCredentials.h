// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_ITCH_IO_ENABLED
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSAuth/OnlineExternalCredentials.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_ITCH_IO_ENABLED

namespace REDPOINT_EOS_FILE_NS_ID(3076326616, Redpoint::EOS::Platform::Integration::ItchIo::Auth)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Auth;

class FItchIoExternalCredentials : public IOnlineExternalCredentials
{
private:
    FString ApiKey;
    TMap<FString, FString> AuthAttributes;

public:
    FItchIoExternalCredentials(const FString &InApiKey, const TMap<FString, FString> &InAuthAttributes);
    UE_NONCOPYABLE(FItchIoExternalCredentials);
    virtual ~FItchIoExternalCredentials() override = default;

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
};

}

namespace Redpoint::EOS::Platform::Integration::ItchIo::Auth
{
REDPOINT_EOS_FILE_NS_EXPORT(3076326616, Redpoint::EOS::Platform::Integration::ItchIo::Auth, FItchIoExternalCredentials)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()
