// Copyright June Rhodes. All Rights Reserved.

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && EOS_GOG_ENABLED

#include "RedpointEOSAuth/OnlineExternalCredentials.h"
#include "RedpointEOSPlatformIntegrationGOG/Logging.h"

namespace Redpoint::EOS::Platform::Integration::GOG
{
using namespace ::Redpoint::EOS::Auth;

class FGOGExternalCredentials : public IOnlineExternalCredentials
{
private:
    FString SessionTicket;
    TMap<FString, FString> AuthAttributes;

    void OnEncryptedAppTicketResult(
        bool bWasSuccessful,
        FString EncodedAppTicket,
        FOnlineExternalCredentialsRefreshComplete OnComplete);

public:
    FGOGExternalCredentials(const FString &InAppTicket, const TMap<FString, FString> &InAuthAttributes);
    virtual ~FGOGExternalCredentials() = default;
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

#endif