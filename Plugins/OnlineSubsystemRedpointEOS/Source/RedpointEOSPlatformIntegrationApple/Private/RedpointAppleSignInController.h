// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_APPLE_ENABLED
#import <AuthenticationServices/AuthenticationServices.h>
#import <Foundation/Foundation.h>
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_APPLE_ENABLED

REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(3260454029, Redpoint::EOS::Platform::Integration::Apple::Auth, FAppleSignInControllerWrapper)

/**
 * The authorization controller which handles "Sign in with Apple" functionality.
 */
@interface FRedpointAppleSignInController
    : NSObject <ASAuthorizationControllerDelegate, ASAuthorizationControllerPresentationContextProviding>
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wobjc-interface-ivars"
    ::Redpoint::EOS::Platform::Integration::Apple::Auth::FAppleSignInControllerWrapper *appleSignInControllerWrapper;
#pragma clang diagnostic pop
}
- (instancetype)initWithControllerWrapper:(::Redpoint::EOS::Platform::Integration::Apple::Auth::FAppleSignInControllerWrapper *)ControllerWrapper;
@end

#endif

REDPOINT_EOS_CODE_GUARD_END()
