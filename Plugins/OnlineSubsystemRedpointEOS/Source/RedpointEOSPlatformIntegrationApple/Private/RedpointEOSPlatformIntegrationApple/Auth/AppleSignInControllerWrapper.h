// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_APPLE_ENABLED
#include "RedpointAppleSignInController.h"
#import <Foundation/Foundation.h>
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_APPLE_ENABLED

namespace REDPOINT_EOS_FILE_NS_ID(3260454029, Redpoint::EOS::Platform::Integration::Apple::Auth)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Platform::Integration::Apple::Auth;

class FAppleSignInControllerWrapper : public TSharedFromThis<FAppleSignInControllerWrapper>
{
public:
    typedef TDelegate<void(
        const FError& Error,
        const TOptional<FString>& AuthorizationCode)> FOnCompleteDelegate;
    typedef TDelegate<void(
        NSString *user,
        NSData *authorizationCode,
        NSString *email,
        NSPersonNameComponents *fullName,
        NSError *error)> FOnNativeCallbackDelegate;

private:
    FOnCompleteDelegate OnComplete;
    FOnNativeCallbackDelegate OnNativeCallback;
    FRedpointAppleSignInController *Controller;

public:
    FAppleSignInControllerWrapper(const FOnCompleteDelegate& InOnComplete);
    UE_NONCOPYABLE(FAppleSignInControllerWrapper);
    ~FAppleSignInControllerWrapper();

    void Start();
    
    FOnNativeCallbackDelegate GetNativeCallback();

private:
    void OnNativeSignInComplete(
        NSString *user,
        NSData *authorizationCode,
        NSString *email,
        NSPersonNameComponents *fullName,
        NSError *error,
        TSharedRef<FAppleSignInControllerWrapper> SelfReference);
};

}

namespace Redpoint::EOS::Platform::Integration::Apple::Auth
{
REDPOINT_EOS_FILE_NS_EXPORT(3260454029, Redpoint::EOS::Platform::Integration::Apple::Auth, FAppleSignInControllerWrapper)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()
