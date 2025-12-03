// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatformIntegrationApple/Auth/AppleExternalCredentials.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_APPLE_ENABLED
#include "RedpointEOSPlatformIntegrationApple/Logging.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_APPLE_ENABLED

namespace REDPOINT_EOS_FILE_NS_ID(3580085967, Redpoint::EOS::Platform::Integration::Apple::Auth)
{
using namespace ::Redpoint::EOS::Platform::Integration::Apple;
using namespace ::Redpoint::EOS::Platform::Integration::Apple::Auth;

FAppleExternalCredentials::FAppleExternalCredentials(
    const FString &InToken)
{
    this->Token = InToken;
}

FText FAppleExternalCredentials::GetProviderDisplayName() const
{
    return NSLOCTEXT("OnlineSubsystemRedpointEOS", "Platform_Apple", "Apple");
}

FString FAppleExternalCredentials::GetType() const
{
    return TEXT("APPLE_ID_TOKEN");
}

FString FAppleExternalCredentials::GetId() const
{
    return TEXT("Anonymous");
}

FString FAppleExternalCredentials::GetToken() const
{
    return this->Token;
}

ESupportedLinkType FAppleExternalCredentials::GetProviderSupportedLinkType(FName PlatformName) const
{
    // We can't link Apple accounts to Epic Games.
    return ESupportedLinkType::None;
}

TMap<FString, FString> FAppleExternalCredentials::GetAuthAttributes() const
{
    return TMap<FString, FString>();
}

FName FAppleExternalCredentials::GetNativeSubsystemName() const
{
    return NAME_None;
}

void FAppleExternalCredentials::Refresh(
    TSoftObjectPtr<UWorld> InWorld,
    int32 LocalUserNum,
    FOnlineExternalCredentialsRefreshComplete OnComplete)
{
    UE_LOG(LogRedpointEOSPlatformIntegrationApple, Verbose, TEXT("Requesting new authorization code for existing user for credential refresh..."));

    // Create the controller wrapper and start the request.
    TSharedRef<FAppleSignInControllerWrapper> ControllerWrapper = MakeShared<FAppleSignInControllerWrapper>(
        FAppleSignInControllerWrapper::FOnCompleteDelegate::CreateSP(
            this,
            &FAppleExternalCredentials::OnSignInComplete,
            OnComplete));
    ControllerWrapper->Start();
}

void FAppleExternalCredentials::OnSignInComplete(
    const FError& Error,
    const TOptional<FString>& AuthorizationCode,
    FOnlineExternalCredentialsRefreshComplete OnComplete)
{
    if (Error.WasSuccessful())
    {
        // Successful refresh.
        UE_LOG(LogRedpointEOSPlatformIntegrationApple, Verbose, TEXT("Successfully obtained new authorization code for existing user for credential refresh."));

        this->Token = AuthorizationCode.GetValue();
        OnComplete.ExecuteIfBound(true);
    }
    else
    {
        // Failed to refresh.
        UE_LOG(LogRedpointEOSPlatformIntegrationApple, Error, TEXT("Failed to retrieve new authorization code upon credential refresh: %s"), *Error.ToLogString());
        
        OnComplete.ExecuteIfBound(false);
    }
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()
