// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatformIntegrationApple/Services/AppleRuntimePlatformAuthService.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_APPLE_ENABLED
#include "IOS/IOSAppDelegate.h"
#include "RedpointEOSAuth/AuthenticationConstants.h"
#include "RedpointEOSPlatformIntegrationApple/Auth/AppleExternalCredentials.h"
#include "RedpointEOSPlatformIntegrationApple/Auth/AppleSignInControllerWrapper.h"
#include "RedpointEOSPlatformIntegrationApple/Logging.h"
#include "RedpointEOSAuth/AuthenticationGraphState.h"
#import <AuthenticationServices/AuthenticationServices.h>
#import <Foundation/Foundation.h>
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_APPLE_ENABLED

namespace REDPOINT_EOS_FILE_NS_ID(3138795473, Redpoint::EOS::Platform::Integration::Apple::Services)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Platform::Integration::Apple;
using namespace ::Redpoint::EOS::Platform::Integration::Apple::Auth;

FName FAppleRuntimePlatformAuthService::GetPlatformName() const
{
    return FName(TEXT("Apple"));
}

int32 FAppleRuntimePlatformAuthService::GetSelectionScore(
    const FRuntimePlatformServiceCallContextRef &CallContext) const
{
    // We always offer Apple authentication if this code is compiled on iOS, because there are no other authentication
    // mechanisms on iOS (except device ID which doesn't using runtime platform integrations, and Epic Games
    // authentication which is a cross-platform account provider).
    UE_LOG(
        LogRedpointEOSPlatformIntegrationApple,
        Verbose,
        TEXT("Offering Apple auth service because this build is running on iOS."));
    return 1000;
}

void FAppleRuntimePlatformAuthService::EnsureSignedInOnLocalPlatform(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const FOnEnsureSignedInOnLocalPlatform &OnComplete)
{
    // If we are already signed in, there is nothing to do.
    if (this->SignedInUser.IsValid())
    {
        OnComplete.ExecuteIfBound(Errors::Success());
        return;
    }

    UE_LOG(LogRedpointEOSPlatformIntegrationApple, Verbose, TEXT("Requesting authorization code to sign in new user on Apple device..."));

    // Create the controller wrapper and start the request.
    TSharedRef<FAppleSignInControllerWrapper> ControllerWrapper = MakeShared<FAppleSignInControllerWrapper>(
        FAppleSignInControllerWrapper::FOnCompleteDelegate::CreateSP(
            this,
            &FAppleRuntimePlatformAuthService::OnSignInComplete,
            CallContext,
            OnComplete));
    ControllerWrapper->Start();
}

void FAppleRuntimePlatformAuthService::OnSignInComplete(
    const FError& Error,
    const TOptional<FString>& AuthorizationCode,
    FRuntimePlatformServiceCallContextRef CallContext,
    FOnEnsureSignedInOnLocalPlatform OnComplete)
{
    if (Error.WasSuccessful())
    {
        // Successful login.
        UE_LOG(LogRedpointEOSPlatformIntegrationApple, Verbose, TEXT("Successfully signed in local user on Apple device."));

        this->SignedInUser = MakeShared<FAppleExternalCredentials>(AuthorizationCode.GetValue());
        OnComplete.ExecuteIfBound(Errors::Success());
    }
    else
    {
        // Failed to login.
        UE_LOG(LogRedpointEOSPlatformIntegrationApple, Error, TEXT("Failed to sign in local user on Apple device: %s"), *Error.ToLogString());
        
        OnComplete.ExecuteIfBound(Error);
    }
}

void FAppleRuntimePlatformAuthService::IsOnlinePlayPermitted(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const FOnIsOnlinePlayPermitted &OnComplete)
{
    // We are always permitted to play online on Apple devices.
    OnComplete.ExecuteIfBound(Errors::Success());
}

void FAppleRuntimePlatformAuthService::GetCredentialsFromLocalPlatformDuringAuthenticationGraph(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const TSharedRef<FAuthenticationGraphState> &State,
    const FOnGetCredentialsFromLocalPlatformDuringAuthenticationGraph &OnComplete)
{
    if (this->SignedInUser.IsValid())
    {
        State->AvailableExternalCredentials.Add(this->SignedInUser.ToSharedRef());
        OnComplete.ExecuteIfBound(Errors::Success());
    }
    else
    {
        OnComplete.ExecuteIfBound(Errors::UnexpectedError());
    }
}

};

#endif

REDPOINT_EOS_CODE_GUARD_END()
