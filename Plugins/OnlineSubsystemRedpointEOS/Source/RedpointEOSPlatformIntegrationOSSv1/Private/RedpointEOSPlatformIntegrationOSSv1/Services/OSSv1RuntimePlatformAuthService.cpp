// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatformIntegrationOSSv1/Services/OSSv1RuntimePlatformAuthService.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "Interfaces/OnlineIdentityInterface.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "RedpointEOSAuth/AuthenticationGraphState.h"
#include "RedpointEOSAuth/ExternalCredentialType.h"
#include "RedpointEOSAuth/OnlineProvidedCredentials.h"
#include "RedpointEOSPlatformIntegrationOSSv1/Auth/OnlineSubsystemExternalCredentials.h"
#include "RedpointEOSPlatformIntegrationOSSv1/Caller/SafeOSSv1Caller.h"
#include "RedpointEOSPlatformIntegrationOSSv1/Logging.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3619559314, Redpoint::EOS::Platform::Integration::OSSv1::Services)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Auth;
using namespace ::Redpoint::EOS::Platform::Integration::OSSv1;
using namespace ::Redpoint::EOS::Platform::Integration::OSSv1::Auth;
using namespace ::Redpoint::EOS::Platform::Integration::OSSv1::Caller;

FOSSv1RuntimePlatformAuthService::FOSSv1RuntimePlatformAuthService(
    FName InPlatformName,
    EOS_EExternalCredentialType InObtainedCredentialType,
    const FString &InAuthenticatedWithValue,
    const FString &InTokenAuthAttributeName)
    : PlatformName(InPlatformName)
    , ObtainedCredentialType(InObtainedCredentialType)
    , AuthenticatedWithValue(InAuthenticatedWithValue)
    , TokenAuthAttributeName(InTokenAuthAttributeName)
{
}

void FOSSv1RuntimePlatformAuthService::GetInterfaces(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    TSharedPtr<IOnlineIdentity> &OutIdentityInterface,
    TSharedPtr<IOnlineExternalUI> &OutExternalUIInterface) const
{
    // Get the target online subsystem, and the relevant interfaces, falling back to 'Redpoint<name>' when it's not
    // natively available.
    {
        auto *OSSInstance = Online::GetSubsystem(CallContext->World.Get(), this->PlatformName);
        if (OSSInstance != nullptr)
        {
            if (!OutIdentityInterface.IsValid())
            {
                OutIdentityInterface = OSSInstance->GetIdentityInterface();
            }
            if (!OutExternalUIInterface.IsValid())
            {
                OutExternalUIInterface = OSSInstance->GetExternalUIInterface();
            }
        }
    }
    {
        auto *OSSInstance = Online::GetSubsystem(
            CallContext->World.Get(),
            FName(*FString::Printf(TEXT("Redpoint%s"), *this->PlatformName.ToString())));
        if (OSSInstance != nullptr)
        {
            if (!OutIdentityInterface.IsValid())
            {
                OutIdentityInterface = OSSInstance->GetIdentityInterface();
            }
            if (!OutExternalUIInterface.IsValid())
            {
                OutExternalUIInterface = OSSInstance->GetExternalUIInterface();
            }
        }
    }
}

FName FOSSv1RuntimePlatformAuthService::GetPlatformName() const
{
    return this->PlatformName;
}

int32 FOSSv1RuntimePlatformAuthService::GetSelectionScore(
    const FRuntimePlatformServiceCallContextRef &CallContext) const
{
    UE_LOG(
        LogRedpointEOSPlatformIntegrationOSSv1,
        Warning,
        TEXT("OSSv1 auth service is returning GetSelectionScore of 100. This implementation should be overridden!"));
    return 100;
}

void FOSSv1RuntimePlatformAuthService::EnsureSignedInOnLocalPlatform(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const FOnEnsureSignedInOnLocalPlatform &OnComplete)
{
    TSharedPtr<IOnlineIdentity> IdentityInterface = nullptr;
    TSharedPtr<IOnlineExternalUI> ExternalUIInterface = nullptr;
    this->GetInterfaces(CallContext, IdentityInterface, ExternalUIInterface);

    if (!IdentityInterface.IsValid())
    {
        OnComplete.ExecuteIfBound(Errors::UnexpectedError(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("Could not authenticate with online subsystem, because the subsystem was not available. Check that it "
                 "is enabled in your DefaultEngine.ini file.")));
        return;
    }

    // If the specified controller isn't signed in at all, we need to use the external UI interface to get them to go
    // through the login process.
    //
    // @note: Google implements external UI, but it always fails on Android because it's not necessary to call it.
    if (!this->PlatformName.IsEqual(GOOGLE_SUBSYSTEM) &&
        !IdentityInterface->GetUniquePlayerId(CallContext->LocalUserNum).IsValid() && ExternalUIInterface.IsValid())
    {
        UE_LOG(
            LogRedpointEOSPlatformIntegrationOSSv1,
            Verbose,
            TEXT("Local user ID is not valid, requesting sign in via external UI if possible..."));
        FSafeOSSv1Caller::ShowLoginUI(
            ExternalUIInterface.ToSharedRef(),
            CallContext->LocalUserNum,
            true /* Only allow online profiles */,
            false /* Do not show Skip button */,
            FOnLoginUIClosedDelegate::CreateSP(
                this->AsShared(),
                &FOSSv1RuntimePlatformAuthService::OnLoginUIClosed,
                CallContext,
                OnComplete));
    }
    else if (IdentityInterface->GetUniquePlayerId(CallContext->LocalUserNum).IsValid())
    {
        UE_LOG(
            LogRedpointEOSPlatformIntegrationOSSv1,
            Verbose,
            TEXT("Local user has a valid unique net ID, assuming that external UI doesn't need to be called..."));
        OnComplete.ExecuteIfBound(Errors::Success());
    }
    else
    {
        // Otherwise if the user is already signed in externally, proceed with normal Login call immediately.
        this->StartLoginCall(CallContext, OnComplete);
    }
}

void FOSSv1RuntimePlatformAuthService::OnLoginUIClosed(
    TSharedPtr<const FUniqueNetId> UniqueId,
    const int ControllerIndex,
    const FOnlineError &Error,
    FRuntimePlatformServiceCallContextRef CallContext,
    FOnEnsureSignedInOnLocalPlatform OnComplete) const
{
    if (!Error.bSucceeded)
    {
        UE_LOG(
            LogRedpointEOSPlatformIntegrationOSSv1,
            Error,
            TEXT("Unable to sign user in through online subsystem prompt."));
        OnComplete.ExecuteIfBound(Errors::AccessDenied(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("Unable to sign user in through online subsystem prompt.")));
        return;
    }

    this->StartLoginCall(CallContext, OnComplete);
}

void FOSSv1RuntimePlatformAuthService::StartLoginCall(
    FRuntimePlatformServiceCallContextRef CallContext,
    FOnEnsureSignedInOnLocalPlatform OnComplete) const
{
    TSharedPtr<IOnlineIdentity> IdentityInterface = nullptr;
    TSharedPtr<IOnlineExternalUI> ExternalUIInterface = nullptr;
    this->GetInterfaces(CallContext, IdentityInterface, ExternalUIInterface);

    if (!IdentityInterface.IsValid())
    {
        OnComplete.ExecuteIfBound(Errors::UnexpectedError(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("Could not authenticate with online subsystem, because the subsystem was not available. Check that it "
                 "is enabled in your DefaultEngine.ini file.")));
        return;
    }

    // If the local user isn't signed into the online subsystem, we need them to sign in first before we can get the
    // auth token.
    if (IdentityInterface->GetLoginStatus(CallContext->LocalUserNum) != ELoginStatus::LoggedIn ||
        IdentityInterface->GetAuthToken(CallContext->LocalUserNum).IsEmpty())
    {
        FOnlineAccountCredentials EmptyCreds;
        FSafeOSSv1Caller::Login(
            IdentityInterface.ToSharedRef(),
            CallContext->LocalUserNum,
            EmptyCreds,
            FOnLoginCompleteDelegate::CreateSP(
                this->AsShared(),
                &FOSSv1RuntimePlatformAuthService::OnLoginComplete,
                CallContext,
                OnComplete));
        return;
    }

    // User is now authenticated.
    OnComplete.ExecuteIfBound(Errors::Success());
}

void FOSSv1RuntimePlatformAuthService::OnLoginComplete(
    int32 LocalUserNum,
    bool bWasSuccessful,
    const FUniqueNetId &UserId,
    const FString &Error,
    FRuntimePlatformServiceCallContextRef CallContext,
    FOnEnsureSignedInOnLocalPlatform OnComplete) const
{
    if (LocalUserNum != CallContext->LocalUserNum)
    {
        // This callback is not for us.
        return;
    }

    if (!bWasSuccessful)
    {
        UE_LOG(
            LogRedpointEOSPlatformIntegrationOSSv1,
            Error,
            TEXT("Could not authenticate with online subsystem, subsystem Login() operation failed: %s"),
            *Error);
        OnComplete.ExecuteIfBound(Errors::AccessDenied(
            ANSI_TO_TCHAR(__FUNCTION__),
            FString::Printf(
                TEXT("Could not authenticate with online subsystem, subsystem Login() operation failed: %s."),
                *Error)));
        return;
    }

    // User is now authenticated.
    OnComplete.ExecuteIfBound(Errors::Success());
}

void FOSSv1RuntimePlatformAuthService::IsOnlinePlayPermitted(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const FOnIsOnlinePlayPermitted &OnComplete)
{
    TSharedPtr<IOnlineIdentity> IdentityInterface = nullptr;
    TSharedPtr<IOnlineExternalUI> ExternalUIInterface = nullptr;
    this->GetInterfaces(CallContext, IdentityInterface, ExternalUIInterface);

    if (!IdentityInterface.IsValid())
    {
        OnComplete.ExecuteIfBound(Errors::UnexpectedError(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("Could not authenticate with online subsystem, because the subsystem was not available. Check that it "
                 "is enabled in your DefaultEngine.ini file.")));
        return;
    }

    FUniqueNetIdPtr PlayerId = IdentityInterface->GetUniquePlayerId(CallContext->LocalUserNum);
    if (!PlayerId.IsValid())
    {
        UE_LOG(
            LogRedpointEOSPlatformIntegrationOSSv1,
            Error,
            TEXT("Expected player to be signed in at the point of user privilege checks."));
        OnComplete.ExecuteIfBound(Errors::InvalidUser(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("Expected player to be signed in at the point of user privilege checks.")));
        return;
    }

    IdentityInterface->GetUserPrivilege(
        *PlayerId,
        EUserPrivileges::CanPlayOnline,
        IOnlineIdentity::FOnGetUserPrivilegeCompleteDelegate::CreateSPLambda(
            this,
            [CallContext,
             OnComplete](const FUniqueNetId &LocalUserId, EUserPrivileges::Type Privilege, uint32 PrivilegeResult) {
                if (PrivilegeResult == 0)
                {
                    OnComplete.ExecuteIfBound(Errors::Success());
                }
                else
                {
                    UE_LOG(
                        LogRedpointEOSPlatformIntegrationOSSv1,
                        Error,
                        TEXT("You do not have permission to play online."));
                    OnComplete.ExecuteIfBound(Errors::AccessDenied(
                        ANSI_TO_TCHAR(__FUNCTION__),
                        TEXT("You do not have permission to play online.")));
                }
            })
#if REDPOINT_EOS_UE_5_4_OR_LATER
            ,
        EShowPrivilegeResolveUI::Show
#endif
    );
}

void FOSSv1RuntimePlatformAuthService::GetCredentialsFromLocalPlatformDuringAuthenticationGraph(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const TSharedRef<FAuthenticationGraphState> &State,
    const FOnGetCredentialsFromLocalPlatformDuringAuthenticationGraph &OnComplete)
{
    TSharedPtr<IOnlineIdentity> IdentityInterface = nullptr;
    TSharedPtr<IOnlineExternalUI> ExternalUIInterface = nullptr;
    this->GetInterfaces(CallContext, IdentityInterface, ExternalUIInterface);

    if (!IdentityInterface.IsValid())
    {
        OnComplete.ExecuteIfBound(Errors::UnexpectedError(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("Could not authenticate with online subsystem, because the subsystem was not available. Check that it "
                 "is enabled in your DefaultEngine.ini file.")));
        return;
    }

    FOnlineProvidedCredentials Credentials;
    Credentials.Type = FExternalCredentialType::ToString(this->ObtainedCredentialType);
    Credentials.Token = IdentityInterface->GetAuthToken(CallContext->LocalUserNum);
    Credentials.Id = IdentityInterface->GetPlayerNickname(CallContext->LocalUserNum);
    if (Credentials.Id.IsEmpty())
    {
        Credentials.Id = TEXT("Anonymous");
    }

    UE_LOG(
        LogRedpointEOSPlatformIntegrationOSSv1,
        Verbose,
        TEXT("Authenticating with OSS auth token: %s"),
        *Credentials.Token);

    State->AvailableExternalCredentials.Add(MakeShared<FOnlineSubsystemExternalCredentials>(
        this->PlatformName,
        this->ObtainedCredentialType,
        Credentials,
        this->AuthenticatedWithValue,
        this->TokenAuthAttributeName));
    OnComplete.ExecuteIfBound(Errors::Success());
}

}; // namespace REDPOINT_EOS_FILE_NS_ID(3619559314,Redpoint::EOS::Platform::Integration::OSSv1::Services)

#endif

REDPOINT_EOS_CODE_GUARD_END()