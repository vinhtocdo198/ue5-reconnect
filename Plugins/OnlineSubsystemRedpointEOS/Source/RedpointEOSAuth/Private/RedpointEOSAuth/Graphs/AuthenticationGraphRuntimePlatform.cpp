// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/Graphs/AuthenticationGraphRuntimePlatform.h"

#include "RedpointEOSAuth/AuthenticationGraphRegistry.h"
#include "RedpointEOSAuth/Nodes/AuthenticationGraphNodeConditional.h"
#include "RedpointEOSAuth/Nodes/AuthenticationGraphNodeUntil_Forever.h"
#include "RedpointEOSAuth/Nodes/AuthenticationGraphNodeUntil_LoginComplete.h"
#include "RedpointEOSAuth/Nodes/BailIfAlreadyAuthenticatedNode.h"
#include "RedpointEOSAuth/Nodes/BailIfNotExactlyOneExternalCredentialNode.h"
#include "RedpointEOSAuth/Nodes/CheckUserPrivilegesNode.h"
#include "RedpointEOSAuth/Nodes/FailAuthenticationNode.h"
#include "RedpointEOSAuth/Nodes/GatherEOSAccountsWithExternalCredentialsNode.h"
#include "RedpointEOSAuth/Nodes/GetExternalCredentialsFromLocalPlatformNode.h"
#include "RedpointEOSAuth/Nodes/LogUserIntoLocalPlatformNode.h"
#include "RedpointEOSAuth/Nodes/LoginWithSelectedEOSAccountNode.h"
#include "RedpointEOSAuth/Nodes/PromptToSignInOrCreateAccountNode.h"
#include "RedpointEOSAuth/Nodes/SelectCrossPlatformAccountNode.h"
#include "RedpointEOSAuth/Nodes/SelectOnlyEOSCandidateNode.h"
#include "RedpointEOSAuth/Nodes/SelectSingleContinuanceTokenEOSCandidateNode.h"
#include "RedpointEOSAuth/Nodes/SelectSingleSuccessfulEOSAccountNode.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3215911224, Redpoint::EOS::Auth::Graphs)
{
using namespace ::Redpoint::EOS::Auth;
using namespace ::Redpoint::EOS::Auth::Nodes;

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

static bool Condition_CanUpgradeToCrossPlatformAccount(const FAuthenticationGraphState &State)
{
    return State.ExistingUserId.IsValid() && !State.ExistingCrossPlatformAccountId.IsValid() &&
           State.CrossPlatformAccountProvider.IsValid();
}

TSharedRef<FAuthenticationGraphNode> FAuthenticationGraphRuntimePlatform::CreateGraph(
    const TSharedRef<FAuthenticationGraphState> &InitialState) const
{
    if (InitialState->CrossPlatformAccountProvider.IsValid())
    {
        if (InitialState->Config->GetRequireCrossPlatformAccount())
        {
            return MakeShared<FAuthenticationGraphNodeUntil_Forever>()
                ->Add(MakeShared<FBailIfAlreadyAuthenticatedNode>())
                ->Add(MakeShared<FLogUserIntoLocalPlatformNode>())
                ->Add(MakeShared<FCheckUserPrivilegesNode>())
                ->Add(MakeShared<FGetExternalCredentialsFromLocalPlatformNode>())
                ->Add(InitialState->CrossPlatformAccountProvider->GetInteractiveAuthenticationSequence())
                ->Add(MakeShared<FSelectCrossPlatformAccountNode>())
                ->Add(MakeShared<FLoginWithSelectedEOSAccountNode>())
                // Also link the credential directly onto the EOS account, so that if the developer ever turns off Epic
                // Games, players can still access their accounts.
                ->Add(MakeShared<FGatherEOSAccountsWithExternalCredentialsNode>())
                ->Add(InitialState->CrossPlatformAccountProvider
                          ->GetLinkUnusedExternalCredentialsToCrossPlatformAccountSequence());
        }
        else
        {
            auto SignIn =
                MakeShared<FAuthenticationGraphNodeUntil_Forever>()
                    ->Add(MakeShared<FLogUserIntoLocalPlatformNode>())
                    ->Add(MakeShared<FCheckUserPrivilegesNode>())
                    ->Add(MakeShared<FGetExternalCredentialsFromLocalPlatformNode>())
                    // We *must* have valid external credentials at this point; the default implementation of
                    // FGetExternalCredentialsFromOnlineSubsystemNode enforces this, but double check in case the
                    // OverrideGetCredentialsNode implementation does not.
                    ->Add(MakeShared<FBailIfNotExactlyOneExternalCredentialNode>())
                    // Try to sign into a cross-platform account using those credentials.
                    ->Add(InitialState->CrossPlatformAccountProvider->GetNonInteractiveAuthenticationSequence(true))
                    ->Add(
                        MakeShared<FAuthenticationGraphNodeConditional>()
                            // If we have a cross-platform account at this point, the cross-platform provider will have
                            // also already authenticated it against EOS, so we can select the single remaining node and
                            // finish login.
                            //
                            // NOTE: We do not implicitly sign in with Epic Games accounts that *do not* have an EOS
                            // account here. That's because you can run into this situation:
                            // - You play the game on a console, without signing into an Epic Games account. You
                            //   therefore have an EOS account without cross-platform linkage.
                            // - You then play the game on Steam. You have your Steam account linked to an Epic Games
                            //   account, so we're able to sign into the Epic Games account implicitly. But you *don't*
                            //   have an EOS account associated with this Epic Games account. Implicitly creating an EOS
                            //   account for the Epic Games account here would give you two EOS accounts, which we don't
                            //   want.
                            // It's also possible to run into this situation if you:
                            // - Play the game on Steam, without connecting an Epic Games account.
                            // - At some point, you link your Epic Games account to your Steam account outside of this
                            // game.
                            // - Next time you play the game, you would implicitly get a new EOS account with the Epic
                            //   Games account instead of being able to continue to use your Steam-only EOS account.
                            ->If(
                                FAuthenticationGraphCondition::CreateStatic(
                                    &FAuthenticationGraph::Condition_CrossPlatformAccountIsValidWithPUID),
                                MakeShared<FAuthenticationGraphNodeUntil_Forever>()
                                    ->Add(MakeShared<FSelectCrossPlatformAccountNode>())
                                    ->Add(MakeShared<FLoginWithSelectedEOSAccountNode>()))
                            // We could not implicitly sign into a cross-platform account. Try implicitly signing them
                            // into a platform-specific account first, as they might have created a non-cross-platform
                            // account and have not yet upgraded to a cross-platform account.
                            ->Else(
                                MakeShared<FAuthenticationGraphNodeUntil_Forever>()
                                    ->Add(InitialState->CrossPlatformAccountProvider
                                              ->GetNonInteractiveDeauthenticationSequence())
                                    ->Add(MakeShared<FGatherEOSAccountsWithExternalCredentialsNode>())
                                    ->Add(
                                        MakeShared<FAuthenticationGraphNodeConditional>()
                                            // A non-cross-platform account exists for this credential. Sign the user
                                            // into that account.
                                            ->If(
                                                FAuthenticationGraphCondition::CreateStatic(
                                                    &FAuthenticationGraph::Condition_OneSuccessfulCandidate),
                                                MakeShared<FAuthenticationGraphNodeUntil_Forever>()
                                                    ->Add(MakeShared<FSelectSingleSuccessfulEOSAccountNode>())
                                                    ->Add(MakeShared<FLoginWithSelectedEOSAccountNode>()))
                                            // A non-cross-platform account doesn't exist either. Ask the user
                                            // whether they want to sign into a cross-platform account interactively,
                                            // or create a new non-cross-platform account.
                                            ->Else(
                                                MakeShared<FAuthenticationGraphNodeUntil_Forever>()
                                                    ->Add(MakeShared<FPromptToSignInOrCreateAccountNode>())
                                                    ->Add(
                                                        MakeShared<FAuthenticationGraphNodeConditional>()
                                                            // The user wants to sign into an existing account, which
                                                            // means doing an interactive login with the cross-platform
                                                            // provider.
                                                            ->If(
                                                                FAuthenticationGraphCondition::CreateLambda(
                                                                    [](const FAuthenticationGraphState &State) {
                                                                        return State.LastSignInChoice ==
                                                                               EEOSUserInterface_SignInOrCreateAccount_Choice::
                                                                                   SignIn;
                                                                    }),
                                                                MakeShared<FAuthenticationGraphNodeUntil_Forever>()
                                                                    ->Add(
                                                                        InitialState->CrossPlatformAccountProvider
                                                                            ->GetInteractiveOnlyAuthenticationSequence())
                                                                    ->Add(MakeShared<FSelectCrossPlatformAccountNode>(
                                                                        ESelectCrossPlatformAccountMode::
                                                                            ExistingAccountOnly))
                                                                    ->Add(
                                                                        MakeShared<FLoginWithSelectedEOSAccountNode>())
                                                                    ->Add(
                                                                        InitialState->CrossPlatformAccountProvider
                                                                            ->GetLinkUnusedExternalCredentialsToCrossPlatformAccountSequence()))
                                                            // The user wants to create a new non-cross platform
                                                            // account. Select the EOS candidate from earlier that had a
                                                            // continuance token.
                                                            ->Else(
                                                                MakeShared<FAuthenticationGraphNodeUntil_Forever>()
                                                                    ->Add(MakeShared<
                                                                          FSelectSingleContinuanceTokenEOSCandidateNode>())
                                                                    ->Add(MakeShared<
                                                                          FLoginWithSelectedEOSAccountNode>())))))));

            return MakeShared<FAuthenticationGraphNodeConditional>()
                ->If(
                    FAuthenticationGraphCondition::CreateStatic(&FAuthenticationGraph::Condition_Unauthenticated),
                    SignIn)
                ->If(
                    FAuthenticationGraphCondition::CreateStatic(&Condition_CanUpgradeToCrossPlatformAccount),
                    InitialState->CrossPlatformAccountProvider
                        ->GetUpgradeCurrentAccountToCrossPlatformAccountSequence())
                ->Else(MakeShared<FBailIfAlreadyAuthenticatedNode>());
        }
    }
    else
    {
        return MakeShared<FAuthenticationGraphNodeUntil_Forever>()
            ->Add(MakeShared<FBailIfAlreadyAuthenticatedNode>())
            ->Add(MakeShared<FLogUserIntoLocalPlatformNode>())
            ->Add(MakeShared<FCheckUserPrivilegesNode>())
            ->Add(MakeShared<FGetExternalCredentialsFromLocalPlatformNode>())
            // We *must* have valid external credentials at this point; the default implementation of
            // FGetExternalCredentialsFromOnlineSubsystemNode enforces this, but double check in case the
            // OverrideGetCredentialsNode implementation does not.
            ->Add(MakeShared<FBailIfNotExactlyOneExternalCredentialNode>())
            ->Add(MakeShared<FGatherEOSAccountsWithExternalCredentialsNode>())
            ->Add(MakeShared<FSelectOnlyEOSCandidateNode>())
            ->Add(MakeShared<FLoginWithSelectedEOSAccountNode>());
    }
}

void FAuthenticationGraphRuntimePlatform::Register()
{
    FAuthenticationGraphRegistry::Register(
        EOS_AUTH_GRAPH_RUNTIME_PLATFORM,
        NSLOCTEXT("OnlineSubsystemRedpointEOS", "AuthGraph_RuntimePlatform", "Runtime Platform"),
        MakeShared<FAuthenticationGraphRuntimePlatform>());
}

#endif

}

REDPOINT_EOS_CODE_GUARD_END()