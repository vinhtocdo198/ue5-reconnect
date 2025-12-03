// Copyright June Rhodes. All Rights Reserved.

#include "RedpointAppleSignInController.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_APPLE_ENABLED
#include "RedpointEOSPlatformIntegrationApple/Auth/AppleSignInControllerWrapper.h"
#include "IOS/IOSAppDelegate.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_APPLE_ENABLED

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdirect-ivar-access"

@implementation FRedpointAppleSignInController

- (instancetype)initWithControllerWrapper:(::Redpoint::EOS::Platform::Integration::Apple::Auth::FAppleSignInControllerWrapper *)ControllerWrapper
{
    self = [super init];
    if (self)
    {
        appleSignInControllerWrapper = ControllerWrapper;
    }
    return self;
}

- (ASPresentationAnchor)presentationAnchorForAuthorizationController:(ASAuthorizationController *)controller
{
    if ([IOSAppDelegate GetDelegate].Window == nullptr)
    {
        NSLog(@"authorizationController: presentationAnchorForAuthorizationController: error window is NULL");
    }
    return [IOSAppDelegate GetDelegate].Window;
}

- (void)authorizationController:(ASAuthorizationController *)controller
    didCompleteWithAuthorization:(ASAuthorization *)authorization
{
    if ([authorization.credential isKindOfClass:[ASAuthorizationAppleIDCredential class]])
    {
        ASAuthorizationAppleIDCredential *AuthAppleID = (ASAuthorizationAppleIDCredential *)authorization.credential;

        if (appleSignInControllerWrapper != nullptr)
        {
            appleSignInControllerWrapper->GetNativeCallback().ExecuteIfBound(
                [AuthAppleID user],
                [AuthAppleID identityToken],
                [AuthAppleID email],
                [AuthAppleID fullName],
                nullptr);
        }
    }
    else
    {
        // Not the response for the ControllerWrappered auth type
        if (appleSignInControllerWrapper != nullptr)
        {
            appleSignInControllerWrapper->GetNativeCallback().ExecuteIfBound(nullptr, nullptr, nullptr, nullptr, nullptr);
        }
    }
}

- (void)authorizationController:(ASAuthorizationController *)controller didCompleteWithError:(NSError *)error
{
    if (appleSignInControllerWrapper != nullptr)
    {
        appleSignInControllerWrapper->GetNativeCallback().ExecuteIfBound(nullptr, nullptr, nullptr, nullptr, error);
    }
}

@end

#pragma clang diagnostic pop

#endif

REDPOINT_EOS_CODE_GUARD_END()
