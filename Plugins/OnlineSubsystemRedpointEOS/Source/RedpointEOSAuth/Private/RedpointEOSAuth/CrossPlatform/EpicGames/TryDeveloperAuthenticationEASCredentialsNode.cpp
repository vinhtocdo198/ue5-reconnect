// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/CrossPlatform/EpicGames/TryDeveloperAuthenticationEASCredentialsNode.h"

#include "RedpointEOSAuth/AuthenticationGraphState.h"
#include "RedpointEOSAuth/AuthenticationHelpersEpicGames.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGames/SignOutEASAccountNode.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGamesCrossPlatformAccountProvider.h"
#include "RedpointEOSAuth/Logging.h"
#include "RedpointEOSAuth/Nodes/SignOutEOSAccountNode.h"
#include "RedpointEOSConfig/Config.h"
#include "RedpointEOSCore/Legacy/EOSString.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(2536339074, Redpoint::EOS::Auth::CrossPlatform::EpicGames)
{
using namespace ::Redpoint::EOS::API::Auth;
using namespace ::Redpoint::EOS::Core::Utils;
using namespace ::Redpoint::EOS::Auth::CrossPlatform;
using namespace ::Redpoint::EOS::Auth::CrossPlatform::EpicGames;
using namespace ::Redpoint::EOS::Auth::Nodes;

TSharedRef<FExclusiveAsyncOperationQueue> FTryDeveloperAuthenticationEASCredentialsNode::LCTDeveloperOperationQueue =
    MakeShared<FExclusiveAsyncOperationQueue>();

void FTryDeveloperAuthenticationEASCredentialsNode::ExecuteInOperationQueue(
    const FExclusiveAsyncOperationQueue::FOnDone &OnOperationQueueDone,
    TSharedRef<FAuthenticationGraphState> State,
    FAuthenticationGraphNodeOnDone OnDone,
    FString CredentialAddress,
    FString CredentialName)
{
    UE_LOG(
        LogRedpointEOSAuth,
        Verbose,
        TEXT("%p: TryDeveloperAuthenticationEASCredentialsNode: Starting EOS SDK login call..."),
        &State.Get());

    FAuthenticationHelpersEpicGames::Login(
        State->PlatformHandle,
        CredentialAddress,
        CredentialName,
        EOS_ELoginCredentialType::EOS_LCT_Developer,
        (EOS_EExternalCredentialType)0,
        true,
        *State->Config,
        FAuthenticationHelpersEpicGames::FLoginCompletionDelegate::CreateSP(
            this,
            &FTryDeveloperAuthenticationEASCredentialsNode::OnLoginComplete,
            OnOperationQueueDone,
            State,
            OnDone,
            CredentialAddress,
            CredentialName));
}

void FTryDeveloperAuthenticationEASCredentialsNode::OnLoginComplete(
    const FLogin::Result &Data,
    bool bThisCallIsResponsibleForLogout,
    FExclusiveAsyncOperationQueue::FOnDone OnOperationQueueDone,
    TSharedRef<FAuthenticationGraphState> State,
    FAuthenticationGraphNodeOnDone OnDone,
    FString CredentialAddress,
    FString CredentialName)
{
    UE_LOG(
        LogRedpointEOSAuth,
        Verbose,
        TEXT("%p: TryDeveloperAuthenticationEASCredentialsNode: Received EOS SDK login call."),
        &State.Get());

    State->AttemptedDeveloperAuthenticationCredentialNames.Add(CredentialName);

    if (Data.ResultCode != EOS_EResult::EOS_Success || !EOSString_EpicAccountId::IsValid(Data.LocalUserId))
    {
        // Unable to authenticate with developer tool.
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
        OnOperationQueueDone.ExecuteIfBound();
        return;
    }

    if (bThisCallIsResponsibleForLogout)
    {
        // Make sure we sign out of the Epic Games account on error or through normal logout request.
        State->AddLogoutNode(
            MakeShared<FSignOutEASAccountNode>(MakeShared<FEpicGamesCrossPlatformAccountId>(Data.LocalUserId)));
    }

    // Store how we authenticated with Epic.
    State->AddResultUserAuthAttribute("epic.authenticatedWith", "devTool");

    // Set the authenticated Epic account ID into the state.
    State->AuthenticatedCrossPlatformAccountId = MakeShared<FEpicGamesCrossPlatformAccountId>(Data.LocalUserId);
    OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
    OnOperationQueueDone.ExecuteIfBound();
    return;
}

void FTryDeveloperAuthenticationEASCredentialsNode::Execute(
    TSharedRef<FAuthenticationGraphState> State,
    FAuthenticationGraphNodeOnDone OnDone)
{
    check(!EOSString_EpicAccountId::IsValid(State->GetAuthenticatedEpicAccountId()));

    FString CredentialAddress = State->Config->GetDeveloperToolAddress();

#if PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX
    if (CredentialAddress.StartsWith(TEXT("localhost:")))
    {
#if PLATFORM_WINDOWS
        bool bIsDevToolRunning = FPlatformProcess::IsApplicationRunning(TEXT("EOS_DevAuthTool.exe"));
#elif PLATFORM_MAC || PLATFORM_LINUX
        bool bIsDevToolRunning = FPlatformProcess::IsApplicationRunning(TEXT("EOS_DevAuthTool"));
#else
#error Platform is not supported for this code.
#endif
        if (!bIsDevToolRunning)
        {
            // Developer Authentication Tool is not running, don't try to authenticate.
            OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
            return;
        }
    }
#endif

#if WITH_EDITOR
    if (!State->AutomatedTestingEmailAddress.IsEmpty())
    {
        // Prevent the Developer Authentication Tool being used during automated tests.
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
        return;
    }
#endif

    FString CredentialName = this->GetCredentialName(State);
    if (CredentialName.IsEmpty())
    {
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
        return;
    }
    if (State->AttemptedDeveloperAuthenticationCredentialNames.Contains(CredentialName))
    {
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
        return;
    }

    UE_LOG(
        LogRedpointEOSAuth,
        Verbose,
        TEXT("%p: TryDeveloperAuthenticationEASCredentialsNode: Detected EOS Dev Tool, using address '%s' and "
             "credential name '%s'"),
        &State.Get(),
        *CredentialAddress,
        *CredentialName);

    FTryDeveloperAuthenticationEASCredentialsNode::LCTDeveloperOperationQueue->AddToQueue(
        FExclusiveAsyncOperationQueue::FOperation::CreateSP(
            this,
            &FTryDeveloperAuthenticationEASCredentialsNode::ExecuteInOperationQueue,
            State,
            OnDone,
            CredentialAddress,
            CredentialName));
}

void FTryDeveloperAuthenticationEASCredentialsNode::ForceLCTDeveloperInProgressMutexReset()
{
    // Ensures the operation queue doesn't get stuck "in-progress" when the online subsystems shutdown.
    FTryDeveloperAuthenticationEASCredentialsNode::LCTDeveloperOperationQueue =
        MakeShared<FExclusiveAsyncOperationQueue>();
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()