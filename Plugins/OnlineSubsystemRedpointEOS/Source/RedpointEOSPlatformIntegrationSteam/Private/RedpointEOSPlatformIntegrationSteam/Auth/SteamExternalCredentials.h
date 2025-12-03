// Copyright June Rhodes. All Rights Reserved.

#pragma once

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && defined(REDPOINT_EOS_STEAM_ENABLED)

#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSAuth/OnlineExternalCredentials.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSPlatformIntegrationSteam/Auth/SteamCredentialInfo.h"
#include "RedpointEOSPlatformIntegrationSteam/ForwardDecls.h"

namespace REDPOINT_EOS_FILE_NS_ID(4063608760, Redpoint::EOS::Platform::Integration::Steam::Auth)
{
using namespace ::Redpoint::EOS::Auth;
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Platform::Integration::Steam::Auth;

class FSteamExternalCredentials : public IOnlineExternalCredentials
{
private:
    FSteamCredentialInfo CredentialInfo;

    void OnCredentialsRefreshed(
        const FError &ErrorCode,
        FSteamCredentialInfo NewCredentials,
        FOnlineExternalCredentialsRefreshComplete OnComplete);

public:
    FSteamExternalCredentials(const FSteamCredentialInfo &InCredentialInfo);
    UE_NONCOPYABLE(FSteamExternalCredentials);
    virtual ~FSteamExternalCredentials() = default;
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

namespace Redpoint::EOS::Platform::Integration::Steam::Auth
{
REDPOINT_EOS_FILE_NS_EXPORT(4063608760, Redpoint::EOS::Platform::Integration::Steam::Auth, FSteamExternalCredentials)
}

#endif