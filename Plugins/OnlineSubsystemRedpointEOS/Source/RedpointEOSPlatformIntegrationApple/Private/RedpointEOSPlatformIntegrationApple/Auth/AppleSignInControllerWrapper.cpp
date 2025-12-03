// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatformIntegrationApple/Auth/AppleSignInControllerWrapper.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_APPLE_ENABLED
#include "Async/Async.h"
#include "RedpointAppleSignInController.h"
#include "RedpointEOSPlatformIntegrationApple/Logging.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_APPLE_ENABLED

namespace REDPOINT_EOS_FILE_NS_ID(3260454029, Redpoint::EOS::Platform::Integration::Apple::Auth)
{
using namespace ::Redpoint::EOS::Platform::Integration::Apple;

FAppleSignInControllerWrapper::FAppleSignInControllerWrapper(const FOnCompleteDelegate& InOnComplete)
    : OnComplete(InOnComplete)
    , Controller(nullptr)
{
    this->Controller = [[FRedpointAppleSignInController alloc] initWithControllerWrapper:this];
}

FAppleSignInControllerWrapper::~FAppleSignInControllerWrapper()
{
    if (this->Controller != nullptr)
    {
        [this->Controller release];
        this->Controller = nullptr;
    }
}

void FAppleSignInControllerWrapper::Start()
{
    checkf(this->Controller != nullptr, TEXT("This controller wrapper has already been used!"));

    // Set our native callback up (which keeps a reference to ourselves alive during the request).
    this->OnNativeCallback = FOnNativeCallbackDelegate::CreateSP(
        this,
        &FAppleSignInControllerWrapper::OnNativeSignInComplete,
        (TSharedRef<FAppleSignInControllerWrapper>)this->AsShared());

    // Create the request. We don't need additional scopes.
    ASAuthorizationAppleIDProvider *Provider = [ASAuthorizationAppleIDProvider new];
    ASAuthorizationAppleIDRequest *Request = Provider.createRequest;

    // Start the request.
    ASAuthorizationController *AuthorizationController = [[ASAuthorizationController alloc] initWithAuthorizationRequests:@[Request]];
    FRedpointAppleSignInController* SignInController = this->Controller;
    AuthorizationController.delegate = SignInController;
    AuthorizationController.presentationContextProvider = SignInController;
    [AuthorizationController performRequests];
}
    
FAppleSignInControllerWrapper::FOnNativeCallbackDelegate FAppleSignInControllerWrapper::GetNativeCallback()
{
    return this->OnNativeCallback;
}

void FAppleSignInControllerWrapper::OnNativeSignInComplete(
    NSString *user,
    NSData *authorizationCode,
    NSString *email,
    NSPersonNameComponents *fullName,
    NSError *error,
    TSharedRef<FAppleSignInControllerWrapper> SelfReference)
{
    auto OnCompleteCaptured = this->OnComplete;

    if (user != nullptr)
    {
        // Successful login.
        UE_LOG(LogRedpointEOSPlatformIntegrationApple, Verbose, TEXT("Successfully signed in local user on Apple device."));

        // Invoke the callback, capturing all parameters.
        FString AuthorizationCode = FString([[NSString alloc] initWithData:authorizationCode encoding:NSUTF8StringEncoding]);
        AsyncTask(ENamedThreads::GameThread, [OnCompleteCaptured, AuthorizationCode]() {
            OnCompleteCaptured.ExecuteIfBound(Errors::Success(), AuthorizationCode);
        });
    }
    else
    {
        // Failed to login.
        if (error != nullptr)
        {
            UE_LOG(LogRedpointEOSPlatformIntegrationApple, Error, TEXT("Failed to sign in local user on Apple device: %s"), *FString([error localizedDescription]));
            
            // Invoke the callback, capturing all parameters.
            FError Error = Errors::UnexpectedError(
               ANSI_TO_TCHAR(__FUNCTION__),
               FString([error localizedDescription]));
            AsyncTask(ENamedThreads::GameThread, [OnCompleteCaptured, Error]() {
                OnCompleteCaptured.ExecuteIfBound(Error, TOptional<FString>());
            });
        }
        else
        {
            AsyncTask(ENamedThreads::GameThread, [OnCompleteCaptured]() {
                OnCompleteCaptured.ExecuteIfBound(Errors::UnexpectedError(), TOptional<FString>());
            });
        }
    }

    // This may cause the destructor to execute.
    this->OnNativeCallback = FOnNativeCallbackDelegate();
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()
