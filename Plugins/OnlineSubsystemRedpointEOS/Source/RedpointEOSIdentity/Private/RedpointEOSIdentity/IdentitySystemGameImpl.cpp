// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSIdentity/IdentitySystemGameImpl.h"

#include "RedpointEOSAPI/Auth/DeletePersistentAuth.h"
#include "RedpointEOSAuth/AuthenticationGraphRegistry.h"
#include "RedpointEOSAuth/Graphs/AuthenticationGraphAutomatedTesting.h"
#include "RedpointEOSAuth/Graphs/AuthenticationGraphDevAuthTool.h"
#include "RedpointEOSAuth/OnlineProvidedCredentials.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSCore/Module.h"
#include "RedpointEOSCore/Utils/WorldResolution.h"
#include "RedpointEOSIdentity/IdentityHookRegistry.h"
#include "RedpointEOSIdentity/Logging.h"
#include "RedpointEOSIdentity/NullAuthenticationGraphUserInterface.h"
#include "RedpointEOSUserCache/UserCacheSystem.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1586771502, Redpoint::EOS::Identity)
{
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::API::Connect;
using namespace ::Redpoint::EOS::API::Auth;
using namespace ::Redpoint::EOS::Auth;
using namespace ::Redpoint::EOS::UserCache;

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

FIdentitySystemGameImpl::FIdentitySystemGameImpl(
    const FPlatformHandle &InPlatformHandle,
    const FPlatformInstanceEnvironmentRef &InPlatformInstanceEnvironment)
    : FIdentitySystemImpl(InPlatformHandle, InPlatformInstanceEnvironment)
    , OperationQueue()
    , Logouts()
    , ExpectedLogouts()
    , AuthExpirationHandle()
    , LoginStatusChangedHandle()
{
}

TSharedRef<FExclusiveAsyncOperationQueue> FIdentitySystemGameImpl::GetOperationQueue(FIdentityUserSlot UserSlot)
{
    TSharedPtr<FExclusiveAsyncOperationQueue> &UserOperationQueue = this->OperationQueue.FindOrAdd(UserSlot);
    if (!UserOperationQueue.IsValid())
    {
        UserOperationQueue = MakeShared<FExclusiveAsyncOperationQueue>();
    }
    return UserOperationQueue.ToSharedRef();
}

void FIdentitySystemGameImpl::OnAuthExpiration(const API::Connect::FNotifyAuthExpiration::Result &Result)
{
    using namespace ::Redpoint::EOS::Core::Id;

    auto AffectedUser = this->GetUser(GetAccountId(Result.LocalUserId));
    if (AffectedUser.IsValid())
    {
        this->HandleLoginExpiry(AffectedUser.ToSharedRef());
    }
    else
    {
        UE_LOG(
            LogRedpointEOSIdentity,
            Warning,
            TEXT("User %s got auth expiration event, but was not tracked by the identity system."),
            *GetUserIdString(GetAccountId(Result.LocalUserId)));
    }
}

void FIdentitySystemGameImpl::OnLoginStatusChanged(const API::Connect::FNotifyLoginStatusChanged::Result &Result)
{
    using namespace ::Redpoint::EOS::Core::Id;

    EOS_ELoginStatus PreviousStatus = Result.PreviousStatus;
    if (PreviousStatus == EOS_ELoginStatus::EOS_LS_UsingLocalProfile)
    {
        PreviousStatus = EOS_ELoginStatus::EOS_LS_NotLoggedIn;
    }

    EOS_ELoginStatus CurrentStatus = Result.CurrentStatus;
    if (CurrentStatus == EOS_ELoginStatus::EOS_LS_UsingLocalProfile)
    {
        CurrentStatus = EOS_ELoginStatus::EOS_LS_NotLoggedIn;
    }

    if (CurrentStatus == EOS_ELoginStatus::EOS_LS_NotLoggedIn && PreviousStatus == EOS_ELoginStatus::EOS_LS_LoggedIn)
    {
        // The user got signed out, fire appropriate events.
        auto AffectedUser = this->GetUser(GetAccountId(Result.LocalUserId));
        if (AffectedUser.IsValid())
        {
            if (!ExpectedLogouts.Contains(AffectedUser.ToSharedRef()))
            {
                UE_LOG(
                    LogRedpointEOSIdentity,
                    Warning,
                    TEXT("User %s (local user num %d) was unexpectedly signed out, firing events"),
                    *GetUserIdString(AffectedUser->GetUserId()),
                    AffectedUser->GetUserSlot());
                this->GetOperationQueue(AffectedUser->GetUserSlot())
                    ->AddToQueue(FExclusiveAsyncOperationQueue::FOperation::CreateSPLambda(
                        this,
                        [this, AffectedUser](const FExclusiveAsyncOperationQueue::FOnDone &InOnDone) {
                            this->ExecuteLogoutFromSDKEventInQueue(
                                AffectedUser.ToSharedRef(),
                                IIdentitySystem::FOnLogoutComplete::CreateSPLambda(
                                    this,
                                    [InOnDone](FError ErrorCode) {
                                        InOnDone.ExecuteIfBound();
                                    }));
                        }));
            }
            else
            {
                UE_LOG(
                    LogRedpointEOSIdentity,
                    Verbose,
                    TEXT("SDK notified us that user %s was signed out, but it was expected because a logout operation "
                         "is in-progress for that user; ignoring..."),
                    *GetUserIdString(GetAccountId(Result.LocalUserId)));
            }
        }
        else
        {
            UE_LOG(
                LogRedpointEOSIdentity,
                Verbose,
                TEXT("User %s was signed out, but is not associated with any player; ignoring..."),
                *GetUserIdString(GetAccountId(Result.LocalUserId)));
        }
    }
}

void FIdentitySystemGameImpl::HandleLoginExpiry(const FIdentityUserRef &AffectedUser)
{
    using namespace ::Redpoint::EOS::Core::Id;

    if (!this->GetUser(AffectedUser->GetUserSlot()).IsValid() || !this->GetUser(AffectedUser->GetUserId()).IsValid())
    {
        UE_LOG(
            LogRedpointEOSIdentity,
            Error,
            TEXT("Can't refresh credentials for user %d, they are not signed in"),
            AffectedUser->GetUserSlot());
        return;
    }

    UE_LOG(
        LogRedpointEOSIdentity,
        Verbose,
        TEXT("Authentication is about to expire for %s, invoking refresh callback"),
        *GetUserIdString(AffectedUser->GetUserId()));

    TSoftObjectPtr<UWorld> WorldForRefresh =
        Redpoint::EOS::Core::Utils::FWorldResolution::GetWorld(this->PlatformInstanceEnvironment->InstanceName);
    if (!WorldForRefresh.IsValid())
    {
        UE_LOG(
            LogRedpointEOSIdentity,
            Warning,
            TEXT("FWorldResolution could not obtain world for instance name %s, refresh will probably fail!"),
            *this->PlatformInstanceEnvironment->InstanceName.ToString());
    }

    auto Info = MakeShared<FAuthenticationGraphRefreshEOSCredentialsInfo>(
        WorldForRefresh,
        AffectedUser->GetUserSlot(),
        this->PlatformHandle,
        AffectedUser->GetStaticAuthenticationAttributes());
    Info->OnComplete = FAuthenticationGraphRefreshEOSCredentialsComplete::CreateSP(
        this,
        &FIdentitySystemGameImpl::OnLoginExpiryRefreshComplete,
        Info,
        AffectedUser);
    AffectedUser->GetRefreshCallback().ExecuteIfBound(Info);
}

void FIdentitySystemGameImpl::OnLoginExpiryRefreshComplete(
    bool bWasSuccessful,
    TSharedRef<FAuthenticationGraphRefreshEOSCredentialsInfo> Info,
    FIdentityUserRef AffectedUser)
{
    if (bWasSuccessful)
    {
        for (const auto &Remove : Info->DeleteUserAuthAttributes)
        {
            AffectedUser->GetStaticAuthenticationAttributes().Remove(Remove);
        }
        for (const auto &Add : Info->SetUserAuthAttributes)
        {
            AffectedUser->GetStaticAuthenticationAttributes().Add(Add.Key, Add.Value);
        }
    }

    FIdentityHookContext Context(this->AsShared(), this->PlatformHandle);
    for (const auto &Hook : FIdentityHookRegistry::Get())
    {
        Hook->OnPostCredentialRefresh(Context, AffectedUser, bWasSuccessful);
    }
}

void FIdentitySystemGameImpl::RegisterEvents()
{
    this->AuthExpirationHandle = FNotifyAuthExpiration::Register(
        this->PlatformHandle,
        FNotifyAuthExpiration::Options{},
        FNotifyAuthExpiration::EventDelegate::CreateSP(this, &FIdentitySystemGameImpl::OnAuthExpiration));
    this->LoginStatusChangedHandle = FNotifyLoginStatusChanged::Register(
        this->PlatformHandle,
        FNotifyLoginStatusChanged::Options{},
        FNotifyLoginStatusChanged::EventDelegate::CreateSP(this, &FIdentitySystemGameImpl::OnLoginStatusChanged));
}

void FIdentitySystemGameImpl::PerformLogin(
    const FLoginRequest &Request,
    const IIdentitySystem::FOnLoginComplete &OnComplete)
{
    using namespace ::Redpoint::EOS::Core::Id;
    using namespace ::Redpoint::EOS::Auth::Graphs;
    using namespace ::Redpoint::EOS::Core::Utils;

    // Figure out the user interface implementation to use.
    FIdentityHookContext Context(this->AsShared(), this->PlatformHandle);
    TSharedPtr<IAuthenticationGraphUserInterface> UserInterface = nullptr;
    for (const auto &Hook : FIdentityHookRegistry::Get())
    {
        UserInterface = Hook->OnAuthenticationUserInterfaceRequired(Context);
        if (UserInterface.IsValid())
        {
            break;
        }
    }
    if (!UserInterface.IsValid())
    {
        UE_LOG(
            LogRedpointEOSIdentity,
            Error,
            TEXT("No IIdentityHook provides a user interface; the identity system will not route UI widget calls for "
                 "VR/XR! This is likely to be a bug in the plugin."));
        UserInterface = MakeShared<FNullAuthenticationGraphUserInterface>();
    }

    // Set up the authentication graph state.
    auto State = MakeShared<FAuthenticationGraphState>(
        this->PlatformHandle,
        UserInterface.ToSharedRef(),
        Request.UserSlot,
        this->PlatformInstanceEnvironment->InstanceName,
        this->PlatformInstanceEnvironment->Config);
    if (Request.Type == TEXT("AUTOMATED_TESTING"))
    {
#if !UE_BUILD_SHIPPING
        State->AutomatedTestingEmailAddress = Request.Id;
        State->AutomatedTestingPassword = Request.Token;
#endif
        State->ProvidedCredentials = FOnlineProvidedCredentials();
    }
    else
    {
        State->ProvidedCredentials = FOnlineProvidedCredentials();
        State->ProvidedCredentials.Type = Request.Type;
        State->ProvidedCredentials.Id = Request.Id;
        State->ProvidedCredentials.Token = Request.Token;
    }

    // If a user is already signed into this slot, then this is a linking request and we need to provide the existing
    // user information.
    auto ExistingUser = this->GetUser(Request.UserSlot);
    if (ExistingUser.IsValid())
    {
        State->ExistingUserId = GetUniqueNetId(ExistingUser->GetUserId());
        State->ExistingAuthAttributes = ExistingUser->GetStaticAuthenticationAttributes();
        State->ExistingExternalCredentials = ExistingUser->GetExternalCredentials();

        FString EpicAuthenticatedWith;
        if (ExistingUser->TryGetAuthenticationAttributeValue(TEXT("epic.authenticatedWith"), EpicAuthenticatedWith) &&
            EpicAuthenticatedWith == TEXT("devTool"))
        {
            // The user is already signed in (typically due to "login before PIE"), and AutoLogin is being called a
            // second time. The only reason to call AutoLogin a second time is to start a linking flow, and that doesn't
            // make sense if you're already signed into an Epic Games account, so this can only be caused by whatever
            // map is being played on calling AutoLogin in addition to the "login before PIE" doing it.
            //
            // To make sure users don't get misleading errors from AutoLogin here due to link failure, treat this as a
            // login success without doing anything.
            UE_LOG(
                LogRedpointEOSIdentity,
                Warning,
                TEXT("Detected second AutoLogin call while 'login before PIE' is enabled. Treating this as an "
                     "automatic login success rather than starting the account linking flow (which can never succeed "
                     "because you're already signed into an Epic Games account)."))
            OnComplete.ExecuteIfBound(Errors::Success(), ExistingUser);
            return;
        }
    }

    // Determine the cross-platform account provider to use, if any.
    FName CrossPlatformAccountProviderName =
        this->PlatformInstanceEnvironment->Config->GetCrossPlatformAccountProvider();
    if (!CrossPlatformAccountProviderName.IsNone())
    {
        if (!FAuthenticationGraphRegistry::HasCrossPlatformAccountProvider(CrossPlatformAccountProviderName))
        {
            OnComplete.ExecuteIfBound(
                Errors::InvalidParameters(
                    ANSI_TO_TCHAR(__FUNCTION__),
                    FString::Printf(
                        TEXT("Your application is misconfigured. There is no cross platform account provider "
                             "registered with name '%s'."),
                        *CrossPlatformAccountProviderName.ToString())),
                nullptr);
            return;
        }
        else
        {
            State->CrossPlatformAccountProvider =
                FAuthenticationGraphRegistry::GetCrossPlatformAccountProvider(CrossPlatformAccountProviderName);
        }
    }

    // Construct the authentication graph.
    TSharedPtr<const FAuthenticationGraph> Graph = nullptr;
#if !UE_BUILD_SHIPPING
    if (!State->AutomatedTestingEmailAddress.IsEmpty())
    {
        Graph = MakeShared<FAuthenticationGraphAutomatedTesting>();
    }
    else
#endif
        if (State->ProvidedCredentials.Id == TEXT("DEV_TOOL_AUTO_LOGIN"))
    {
#if !UE_BUILD_SHIPPING
        if (!this->PlatformInstanceEnvironment->Config->GetEditorAuthenticationGraph().IsEqual(
                FName(TEXT("AutomatedTesting"))))
        {
#endif
            Graph = MakeShared<FAuthenticationGraphDevAuthTool>();
#if !UE_BUILD_SHIPPING
        }
        else
        {
            UE_LOG(
                LogRedpointEOSIdentity,
                Verbose,
                TEXT("Enabling the user of the AutomatedTesting graph for play-in-editor login."));
            State->AutomatedTestingEmailAddress = FString::Printf(
                TEXT("CreateOnDemand:PIE_%s"),
                *this->PlatformInstanceEnvironment->InstanceName.ToString());
            State->AutomatedTestingPassword = TEXT("0");
            State->ProvidedCredentials = FOnlineProvidedCredentials();
            Graph = MakeShared<FAuthenticationGraphAutomatedTesting>();
        }
#endif
    }
    else
    {
        TSoftObjectPtr<UWorld> World =
            FWorldResolution::GetWorld(this->PlatformInstanceEnvironment->InstanceName, true);
        FName GraphName = this->PlatformInstanceEnvironment->Config->GetAuthenticationGraph();
#if WITH_EDITOR && (!defined(UE_BUILD_SHIPPING) || !UE_BUILD_SHIPPING)
        if (World.IsValid() &&
            (World->WorldType == EWorldType::Editor || World->WorldType == EWorldType::PIE ||
             World->WorldType == EWorldType::EditorPreview || World->WorldType == EWorldType::Inactive))
        {
            GraphName = this->PlatformInstanceEnvironment->Config->GetEditorAuthenticationGraph();
            UE_LOG(
                LogRedpointEOSIdentity,
                Verbose,
                TEXT("Using authentication graph value '%s' because you are running in the editor."),
                *GraphName.ToString());
        }
#endif
        Graph = Redpoint::EOS::Auth::FAuthenticationGraphRegistry::Get(
            GraphName,
            this->PlatformInstanceEnvironment->Config,
            State->ProvidedCredentials,
            World,
            this->PlatformHandle,
            Request.UserSlot);
        if (!Graph.IsValid())
        {
            OnComplete.ExecuteIfBound(
                Errors::InvalidParameters(
                    ANSI_TO_TCHAR(__FUNCTION__),
                    FString::Printf(
                        TEXT("Your application is misconfigured. There is no authentication graph registered with name "
                             "'%s'."),
                        *GraphName.ToString())),
                nullptr);
            return;
        }
    }

    // Execute the authentication graph.
    Graph->Execute(
        State,
        FAuthenticationGraphOnDone::CreateSP(
            StaticCastSharedRef<FIdentitySystemGameImpl>(this->AsShared()),
            &FIdentitySystemGameImpl::OnAuthenticationGraphComplete,
            State,
            Graph.ToSharedRef(),
            OnComplete));
}

void FIdentitySystemGameImpl::OnAuthenticationGraphComplete(
    EAuthenticationGraphNodeResult Result,
    const FAuthenticationGraphOnLogout &OnLogout,
    TSharedRef<FAuthenticationGraphState> State,
    TSharedRef<const FAuthenticationGraph> Graph,
    IIdentitySystem::FOnLoginComplete OnComplete)
{
    using namespace ::Redpoint::EOS::Core::Id;

    if (Result == Redpoint::EOS::Auth::EAuthenticationGraphNodeResult::Continue)
    {
        check(State->ResultUserId.IsValid());

        // Check to see if the user ID we authenticated as is already signed
        // in under a different local user. If they are, this is an error.
        for (const auto &ExistingUser : this->GetUsers())
        {
            if (*GetUniqueNetId(ExistingUser->GetUserId()) == *State->ResultUserId &&
                ExistingUser->GetUserSlot() != State->LocalUserNum)
            {
                // Unwind login state and de-authenticate the EOS/EAS user before returning.
                OnLogout.ExecuteIfBound(FAuthenticationGraphOnLogoutComplete::CreateLambda([OnComplete]() {
                    OnComplete.ExecuteIfBound(
                        Errors::InvalidAuth(
                            ANSI_TO_TCHAR(__FUNCTION__),
                            TEXT("Authentication error: This user is already signed into as a different local "
                                 "user. Each local user must sign into a unique account.")),
                        nullptr);
                }));
                return;
            }
        }

        UE_LOG(LogRedpointEOSIdentity, Verbose, TEXT("Authentication success: %s"), *(State->ResultUserId->ToString()));

        this->PlatformHandle->GetSystem<IUserCacheSystem>()->GetUser(
            GetAccountId(State->ResultUserId.ToSharedRef()),
            GetAccountId(State->ResultUserId.ToSharedRef()),
            IUserCacheSystem::FOnGetUserComplete::CreateSPLambda(
                this,
                [this, State, OnComplete, OnLogout](FError ErrorCode, FUserInfoPtr UserInfo) {
                    if (!ErrorCode.WasSuccessful())
                    {
                        UE_LOG(
                            LogRedpointEOSIdentity,
                            Error,
                            TEXT("Failed to load user information for local user: %s"),
                            *ErrorCode.ToLogString());
                    }
                    else
                    {
                        checkf(
                            UserInfo.IsValid(),
                            TEXT("Expected UserInfo to be valid if there was no error loading user information."));
                    }

                    auto User = FIdentityUser::Create(
                        this->AsShared(),
                        this->PlatformHandle,
                        State->LocalUserNum,
                        GetAccountId(State->ResultUserId.ToSharedRef()),
                        State->ResultNativeSubsystemName,
                        State->ResultRefreshCallback,
                        State->ResultExternalCredentials,
                        State->ResultCrossPlatformAccountId,
                        State->GetResultUserAuthAttributes(),
                        UserInfo);

                    // Store the logout callback so we can use it later.
                    this->Logouts.Add(User, OnLogout);

                    OnComplete.ExecuteIfBound(Errors::Success(), User);
                }));
    }
    else if (Result == Redpoint::EOS::Auth::EAuthenticationGraphNodeResult::Error)
    {
        for (int i = 0; i < State->ErrorMessages.Num(); i++)
        {
            UE_LOG(LogRedpointEOSIdentity, Error, TEXT("Authentication error #%d: %s"), i, *State->ErrorMessages[i]);
        }
        OnComplete.ExecuteIfBound(
            Errors::InvalidAuth(
                ANSI_TO_TCHAR(__FUNCTION__),
                State->ErrorMessages.Num() == 0
                    ? TEXT("AuthenticationGraph did not specify an error. Ensure you add an error message to "
                           "State->ErrorMessages before returning with EAuthenticationGraphNodeResult::Error.")
                    : State->ErrorMessages.Last()),
            nullptr);
        return;
    }
    else
    {
        check(false /* unknown result state */);
        OnComplete.ExecuteIfBound(
            Errors::UnexpectedError(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("Unexpected result code from authentication graph!")),
            nullptr);
        return;
    }
}

void FIdentitySystemGameImpl::PerformLoginEvents(
    const FLoginRequest &Request,
    const FError &Error,
    const FIdentityUserPtr &IdentityUser)
{
    using namespace ::Redpoint::EOS::Core::Id;

    if (IdentityUser.IsValid())
    {
        UE_LOG(
            LogRedpointEOSIdentity,
            Verbose,
            TEXT("Local user %d is now signed in as %s"),
            IdentityUser->GetUserSlot(),
            *GetUserIdString(IdentityUser->GetUserId()));
        this->RebuildAutoCompleteEntries();
    }
}

void FIdentitySystemGameImpl::PerformLogout(
    const FLogoutRequest &Request,
    const FIdentityUserRef &OldIdentityUser,
    const IIdentitySystem::FOnLogoutComplete &OnComplete)
{
    using namespace ::Redpoint::EOS::Core::Id;

    UE_LOG(
        LogRedpointEOSIdentity,
        Verbose,
        TEXT("Local user %d is being signed out (was signed in as %s)"),
        OldIdentityUser->GetUserSlot(),
        *GetUserIdString(OldIdentityUser->GetUserId()));

    this->ExpectedLogouts.Add(OldIdentityUser);

    // If we have a logout delegate for this user, use it to unwind the authentication state,
    // otherwise just return success.
    FAuthenticationGraphOnLogout OnLogout;
    if (this->Logouts.RemoveAndCopyValue(OldIdentityUser, OnLogout) && OnLogout.IsBound())
    {
        OnLogout.ExecuteIfBound(FAuthenticationGraphOnLogoutComplete::CreateSPLambda(
            this,
            [this, OldIdentityUser, OnComplete]() {
                this->ExpectedLogouts.Remove(OldIdentityUser);
                OnComplete.ExecuteIfBound(Errors::Success());
            }));
    }
    else
    {
        this->ExpectedLogouts.Remove(OldIdentityUser);
        OnComplete.ExecuteIfBound(Errors::Success());
    }
}

void FIdentitySystemGameImpl::PerformLogoutEvents(
    const FLogoutRequest &Request,
    const FIdentityUserRef &OldIdentityUser,
    const FError &Error)
{
    if (!Error.WasSuccessful())
    {
        UE_LOG(
            LogRedpointEOSIdentity,
            Verbose,
            TEXT("Local user %d is now signed out"),
            OldIdentityUser->GetUserSlot());
        this->RebuildAutoCompleteEntries();
    }
}

void FIdentitySystemGameImpl::Login(const FLoginRequest &Request, const IIdentitySystem::FOnLoginComplete &OnComplete)
{
    this->GetOperationQueue(Request.UserSlot)
        ->AddToQueue(FExclusiveAsyncOperationQueue::FOperation::CreateSPLambda(
            this,
            [this, Request, OnComplete](const FExclusiveAsyncOperationQueue::FOnDone &InOnDone) {
                this->ExecuteLoginInQueue(
                    Request,
                    IIdentitySystem::FOnLoginComplete::CreateSPLambda(
                        this,
                        [OnComplete, InOnDone](FError ErrorCode, FIdentityUserPtr NewUser) {
                            OnComplete.ExecuteIfBound(ErrorCode, NewUser);
                            InOnDone.ExecuteIfBound();
                        }));
            }));
}

void FIdentitySystemGameImpl::Logout(
    const FLogoutRequest &Request,
    const IIdentitySystem::FOnLogoutComplete &OnComplete)
{
    this->GetOperationQueue(Request.UserSlot)
        ->AddToQueue(FExclusiveAsyncOperationQueue::FOperation::CreateSPLambda(
            this,
            [this, Request, OnComplete](const FExclusiveAsyncOperationQueue::FOnDone &InOnDone) {
                this->ExecuteLogoutInQueue(
                    Request,
                    IIdentitySystem::FOnLogoutComplete::CreateSPLambda(
                        this,
                        [OnComplete, InOnDone](FError ErrorCode) {
                            OnComplete.ExecuteIfBound(ErrorCode);
                            InOnDone.ExecuteIfBound();
                        }));
            }));
}

void FIdentitySystemGameImpl::RegisterAutoCompleteEntries(TArray<FAutoCompleteCommand> &Commands)
{
#if UE_ALLOW_EXEC_DEV
    for (const auto &User : this->GetUsers())
    {
        FAutoCompleteCommand SimulateExpiry;
        SimulateExpiry.Command = FString::Printf(TEXT("SimulateExpiry %d"), User->GetUserSlot());
        SimulateExpiry.Desc = TEXT("Simulate the EOS SDK sending a 'credential refresh required' event, and refresh "
                                   "the user's credentials immediately.");
        Commands.Add(SimulateExpiry);

        FAutoCompleteCommand SimulateLogout;
        SimulateLogout.Command = FString::Printf(TEXT("SimulateSDKLogout %d"), User->GetUserSlot());
        SimulateLogout.Desc = TEXT("Simulate the EOS SDK sending an 'unexpected logout' event from which we can not "
                                   "automatically recover, and proceed to sign out the user.");
        Commands.Add(SimulateLogout);
    }
#endif
}

bool FIdentitySystemGameImpl::Exec_Dev(UWorld *InWorld, const TCHAR *Cmd, FOutputDevice &Ar)
{
    using namespace ::Redpoint::EOS::Core::Id;
    using namespace ::Redpoint::EOS::Core::Utils;

    TSoftObjectPtr<UWorld> OurWorld = FWorldResolution::GetWorld(this->PlatformInstanceEnvironment->InstanceName);
    if (!OurWorld.IsValid())
    {
        UE_LOG(
            LogRedpointEOSIdentity,
            Warning,
            TEXT("FWorldResolution failed to get world for instance %s when command was arriving in ::Exec"),
            *this->PlatformInstanceEnvironment->InstanceName.ToString());
        return false;
    }
    if (OurWorld.Get() != InWorld)
    {
        return false;
    }

    if (FParse::Command(&Cmd, TEXT("SimulateExpiry"), false))
    {
        int32 LocalUserNum = FCString::Atoi(*FParse::Token(Cmd, false));
        auto AffectedUser = this->GetUser(LocalUserNum);
        if (AffectedUser.IsValid())
        {
            Ar.Logf(ELogVerbosity::Verbose, TEXT("Simulating login expiry for local user num %d..."), LocalUserNum);
            this->HandleLoginExpiry(AffectedUser.ToSharedRef());
        }
        else
        {
            Ar.Logf(
                ELogVerbosity::Error,
                TEXT("Unable to simulate login expiry for local user %d because they are not signed in."),
                LocalUserNum);
        }
        return true;
    }

    if (FParse::Command(&Cmd, TEXT("SimulateSDKLogout"), false))
    {
        int32 LocalUserNum = FCString::Atoi(*FParse::Token(Cmd, false));
        auto AffectedUser = this->GetUser(LocalUserNum);
        if (AffectedUser.IsValid())
        {
            Ar.Logf(
                ELogVerbosity::Verbose,
                TEXT("Simulating unexpected logout from SDK for local user num %d..."),
                LocalUserNum);
            this->OnLoginStatusChanged(FNotifyLoginStatusChanged::Result{
                GetProductUserId(AffectedUser->GetUserId()),
                EOS_ELoginStatus::EOS_LS_LoggedIn,
                EOS_ELoginStatus::EOS_LS_NotLoggedIn});
        }
        else
        {
            Ar.Logf(
                ELogVerbosity::Error,
                TEXT("Unable to unexpected logout from SDK for local user %d because they are not signed in."),
                LocalUserNum);
        }
        return true;
    }

    return false;
}

#endif

}

REDPOINT_EOS_CODE_GUARD_END()