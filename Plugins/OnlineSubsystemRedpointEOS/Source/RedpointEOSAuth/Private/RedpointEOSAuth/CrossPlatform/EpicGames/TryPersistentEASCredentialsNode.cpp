// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/CrossPlatform/EpicGames/TryPersistentEASCredentialsNode.h"

#include "RedpointEOSAuth/AuthenticationGraphState.h"
#include "RedpointEOSAuth/AuthenticationHelpersEpicGames.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGames/SignOutEASAccountNode.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGamesCrossPlatformAccountProvider.h"
#include "RedpointEOSAuth/Logging.h"
#include "RedpointEOSConfig/Config.h"
#include "RedpointEOSCore/Legacy/EOSString.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1405438762, Redpoint::EOS::Auth::CrossPlatform::EpicGames)
{
using namespace ::Redpoint::EOS::Auth;
using namespace ::Redpoint::EOS::Auth::CrossPlatform;
using namespace ::Redpoint::EOS::Auth::CrossPlatform::EpicGames;

void FTryPersistentEASCredentialsNode::Execute(
    TSharedRef<FAuthenticationGraphState> State,
    FAuthenticationGraphNodeOnDone OnDone)
{
    using namespace ::Redpoint::EOS::API::Auth;

    check(!EOSString_EpicAccountId::IsValid(State->GetAuthenticatedEpicAccountId()));

    // Load the persistent authentication token if it's set.
    FString AuthenticationToken = TEXT("");
    if (State->Metadata.Contains(EOS_METADATA_EAS_REFRESH_TOKEN))
    {
        AuthenticationToken = State->Metadata[EOS_METADATA_EAS_REFRESH_TOKEN].GetValue<FString>();
    }

    // If this platform doesn't support logging in with no persistent token, or if we don't have a
    // persistent token and we're not logging in for the first user, do not attempt persistent authentication.
    bool bDoNotAttemptOnEmptyRefreshToken =
        State->Metadata.Contains(EOS_METADATA_NO_EMPTY_EAS_REFRESH_TOKEN_ATTEMPT) &&
        State->Metadata[EOS_METADATA_NO_EMPTY_EAS_REFRESH_TOKEN_ATTEMPT].GetValue<bool>();
    if ((bDoNotAttemptOnEmptyRefreshToken || State->LocalUserNum != 0) && AuthenticationToken.IsEmpty())
    {
        // Don't attempt persistent authentication.
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
        return;
    }

    UE_LOG(LogRedpointEOSAuth, Verbose, TEXT("TryPersistentEASCredentialsNode: Attempting persistent authentication"));

    FAuthenticationHelpersEpicGames::Login(
        State->PlatformHandle,
        TEXT(""),
        AuthenticationToken,
        EOS_ELoginCredentialType::EOS_LCT_PersistentAuth,
        (EOS_EExternalCredentialType)0,
        true,
        *State->Config,
        FAuthenticationHelpersEpicGames::FLoginCompletionDelegate::CreateSPLambda(
            this,
            [State, OnDone](const FLogin::Result &Data, bool bThisCallIsResponsibleForLogout) {
                if (Data.ResultCode != EOS_EResult::EOS_Success || !EOSString_EpicAccountId::IsValid(Data.LocalUserId))
                {
                    // Unable to authenticate with persistent token.
                    OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
                    return;
                }

                if (bThisCallIsResponsibleForLogout)
                {
                    // Sign the Epic Games user out on logout or failure.
                    State->AddLogoutNode(MakeShared<FSignOutEASAccountNode>(
                        MakeShared<FEpicGamesCrossPlatformAccountId>(Data.LocalUserId)));
                }

                // Store how we authenticated with Epic.
                State->AddResultUserAuthAttribute("epic.authenticatedWith", "persistent");

                // Set the authenticated Epic account ID into the state.
                State->AuthenticatedCrossPlatformAccountId =
                    MakeShared<FEpicGamesCrossPlatformAccountId>(Data.LocalUserId);
                OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
                return;
            }));
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()