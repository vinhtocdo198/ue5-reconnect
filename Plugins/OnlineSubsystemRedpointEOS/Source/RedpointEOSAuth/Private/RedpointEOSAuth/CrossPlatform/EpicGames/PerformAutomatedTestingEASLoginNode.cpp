// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/CrossPlatform/EpicGames/PerformAutomatedTestingEASLoginNode.h"

#include "RedpointEOSAuth/AuthenticationGraphState.h"
#include "RedpointEOSAuth/AuthenticationHelpersEpicGames.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGames/SignOutEASAccountNode.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGamesCrossPlatformAccountProvider.h"
#include "RedpointEOSConfig/Config.h"
#include "RedpointEOSCore/Legacy/EOSString.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && (!defined(UE_BUILD_SHIPPING) || !UE_BUILD_SHIPPING)

namespace REDPOINT_EOS_FILE_NS_ID(2484283026, Redpoint::EOS::Auth::CrossPlatform::EpicGames)
{
using namespace ::Redpoint::EOS::Auth::CrossPlatform;
using namespace ::Redpoint::EOS::Auth::CrossPlatform::EpicGames;

void FPerformAutomatedTestingEASLoginNode::Execute(
    TSharedRef<FAuthenticationGraphState> State,
    FAuthenticationGraphNodeOnDone OnDone)
{
    using namespace ::Redpoint::EOS::API::Auth;
    
    check(!EOSString_EpicAccountId::IsValid(State->GetAuthenticatedEpicAccountId()));

    if (State->AutomatedTestingEmailAddress.IsEmpty() || State->AutomatedTestingPassword.IsEmpty())
    {
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
        return;
    }

    FAuthenticationHelpersEpicGames::Login(
        State->PlatformHandle,
        State->AutomatedTestingEmailAddress,
        State->AutomatedTestingPassword,
        EOS_ELoginCredentialType::EOS_LCT_Password,
        (EOS_EExternalCredentialType)0,
        false,
        *State->Config,
        FAuthenticationHelpersEpicGames::FLoginCompletionDelegate::CreateSPLambda(
            this,
            [State, OnDone](const FLogin::Result &Data, bool bThisCallIsResponsibleForLogout) {
                if (Data.ResultCode != EOS_EResult::EOS_Success || !EOSString_EpicAccountId::IsValid(Data.LocalUserId))
                {
                    State->ErrorMessages.Add(FString::Printf(
                        TEXT("PerformAutomatedTestingEASLoginNode: Failed to login with automated testing account: %s"),
                        ANSI_TO_TCHAR(EOS_EResult_ToString(Data.ResultCode))));
                    OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
                    return;
                }

                if (bThisCallIsResponsibleForLogout)
                {
                    // Make sure we sign out of the Epic Games account on error or through normal logout request.
                    State->AddLogoutNode(MakeShared<FSignOutEASAccountNode>(
                        MakeShared<FEpicGamesCrossPlatformAccountId>(Data.LocalUserId)));
                }

                // Store how we authenticated with Epic.
                State->AddResultUserAuthAttribute("epic.authenticatedWith", "automatedTesting");

                State->AuthenticatedCrossPlatformAccountId =
                    MakeShared<FEpicGamesCrossPlatformAccountId>(Data.LocalUserId);
                OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
                return;
            }));
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()