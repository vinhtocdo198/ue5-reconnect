// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Auth/Login.h"

#include "RedpointEOSAPI/Templates/Impl/Async.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#if PLATFORM_IOS
#include "IOS/IOSAppDelegate.h"
#import <AuthenticationServices/AuthenticationServices.h>

@interface PresentationContext : NSObject <ASWebAuthenticationPresentationContextProviding>
{
}
@end

@implementation PresentationContext

- (ASPresentationAnchor)presentationAnchorForWebAuthenticationSession:(ASWebAuthenticationSession *)session
{
    if ([IOSAppDelegate GetDelegate].Window == nullptr)
    {
        NSLog(@"authorizationController: presentationAnchorForAuthorizationController: error window is NULL");
    }
    return [IOSAppDelegate GetDelegate].Window;
}

@end

static PresentationContext *PresentationContextProvider = nullptr;
#endif
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Auth
{

REDPOINT_EOSSDK_API_CALL_ASYNC_IMPL(Login);

void FLogin::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    using namespace ::Redpoint::EOS::API::Templates::Util;

    auto &Credentials = Allocator.Allocate<EOS_Auth_Credentials>();
    Credentials.ApiVersion = EOS_AUTH_CREDENTIALS_API_LATEST;
    Credentials.Id = Allocator.AsUtf8(*Options.CredentialId, EAllocationFlags::ReturnNullptrIfEmpty);
    Credentials.Token = Allocator.AsUtf8(*Options.CredentialToken, EAllocationFlags::ReturnNullptrIfEmpty);
    Credentials.Type = *Options.CredentialType;
    Credentials.SystemAuthCredentialsOptions = nullptr;
    Credentials.ExternalType = *Options.CredentialExternalType;

#if PLATFORM_IOS
    auto &IOSOpts = Allocator.Allocate<EOS_IOS_Auth_CredentialsOptions>();
    IOSOpts.ApiVersion = EOS_IOS_AUTH_CREDENTIALSOPTIONS_API_LATEST;
    if (@available(iOS 13.0, *))
    {
        if (PresentationContextProvider != nil)
        {
            [PresentationContextProvider release];
            PresentationContextProvider = nil;
        }
        PresentationContextProvider = [PresentationContext new];
        IOSOpts.PresentationContextProviding = (void *)CFBridgingRetain(PresentationContextProvider);
    }
    else
    {
        IOSOpts.PresentationContextProviding = nullptr;
    }
    Credentials.SystemAuthCredentialsOptions = &IOSOpts;
#endif

    NativeOptions.Credentials = &Credentials;
    NativeOptions.ScopeFlags = *Options.ScopeFlags;
    NativeOptions.LoginFlags = 0;
    if (*Options.bNoUserInterface)
    {
        NativeOptions.LoginFlags |= EOS_LF_NO_USER_INTERFACE;
    }
}

void FLogin::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.ResultCode = NativeResult.ResultCode;
    Result.LocalUserId = NativeResult.LocalUserId;
    if (NativeResult.PinGrantInfo != nullptr)
    {
        FPinGrantInfo Info = {};
        Info.UserCode = Converter.FromUtf8(NativeResult.PinGrantInfo->UserCode);
        Info.VerificationURI = Converter.FromUtf8(NativeResult.PinGrantInfo->VerificationURI);
        Info.ExpiresIn = NativeResult.PinGrantInfo->ExpiresIn;
        Info.VerificationURIComplete = Converter.FromUtf8(NativeResult.PinGrantInfo->VerificationURIComplete);
        Result.PinGrantInfo = Info;
    }
    else
    {
        Result.PinGrantInfo.Reset();
    }
    Result.ContinuanceToken = NativeResult.ContinuanceToken;
    Result.SelectedAccountId = NativeResult.SelectedAccountId;
}

}

REDPOINT_EOS_CODE_GUARD_END()