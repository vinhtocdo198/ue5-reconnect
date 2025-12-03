// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/CrossPlatform/EpicGamesCrossPlatformAccountProvider.h"

#include "RedpointEOSAuth/AuthenticationGraph.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGames/ChainEASResultToEOSNode.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGames/ClearEOSCandidatesNode.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGames/DeletePersistentAuthOnLogoutNode.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGames/FailAuthenticationDueToConflictingAccountsNode.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGames/GatherEASAccountsWithExternalCredentialsNode.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGames/GetEASCTOAForExistingExternalCredentialsNode.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGames/LinkEASContinuanceTokenToExistingAccountNode.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGames/LinkUnconnectedEOSAccountToSignedInEASAccountNode.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGames/PerformAutomatedTestingEASLoginNode.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGames/PerformInteractiveEASLoginNode.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGames/PerformInteractiveLinkExternalCredentialsToEASAccountNode.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGames/SignOutEASCandidateNode.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGames/TryDefaultDeveloperAuthenticationEASCredentialsNode.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGames/TryDeveloperAuthenticationEASCredentialsNode.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGames/TryExchangeCodeAuthenticationNode.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGames/TryPIEDeveloperAuthenticationEASCredentialsNode.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGames/TryPersistentEASCredentialsNode.h"
#include "RedpointEOSAuth/Logging.h"
#include "RedpointEOSAuth/Nodes/AuthenticationGraphNodeConditional.h"
#include "RedpointEOSAuth/Nodes/AuthenticationGraphNodeUntil_CrossPlatformAccountPresent.h"
#include "RedpointEOSAuth/Nodes/AuthenticationGraphNodeUntil_Forever.h"
#include "RedpointEOSAuth/Nodes/FailAuthenticationNode.h"
#include "RedpointEOSAuth/Nodes/LoginWithSelectedEOSAccountNode.h"
#include "RedpointEOSAuth/Nodes/NoopAuthenticationGraphNode.h"
#include "RedpointEOSAuth/Nodes/SelectCrossPlatformAccountNode.h"
#include "RedpointEOSCore/Id/UniqueNetIdEOS.h"
#include "RedpointEOSCore/Legacy/EOSString.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(2396107155, Redpoint::EOS::Auth::CrossPlatform)
{
using namespace ::Redpoint::EOS::Auth::CrossPlatform::EpicGames;
using namespace ::Redpoint::EOS::Auth::Nodes;

FName FEpicGamesCrossPlatformAccountProvider::GetName() const
{
    return EPIC_GAMES_ACCOUNT_ID;
}

TSharedPtr<const FCrossPlatformAccountId> FEpicGamesCrossPlatformAccountProvider::CreateCrossPlatformAccountId(
    const FString &InStringRepresentation) const
{
    return FEpicGamesCrossPlatformAccountId::ParseFromString(InStringRepresentation);
}

TSharedPtr<const FCrossPlatformAccountId> FEpicGamesCrossPlatformAccountProvider::CreateCrossPlatformAccountId(
    uint8 *InBytes,
    int32 InSize) const
{
    FString Data = BytesToString(InBytes, InSize);
    return FEpicGamesCrossPlatformAccountId::ParseFromString(Data);
}

static bool HasContinuanceToken(const FAuthenticationGraphState &State)
{
    return State.EASExternalContinuanceToken != nullptr;
}

static bool HasEpicGamesAccount(const FAuthenticationGraphState &State)
{
    return State.AuthenticatedCrossPlatformAccountId.IsValid();
}

static bool CrossPlatformEOSCandidateMatchesExistingAccount(const FAuthenticationGraphState &State)
{
    for (const auto &Candidate : State.EOSCandidates)
    {
        if (Candidate.Type == EAuthenticationGraphEOSCandidateType::CrossPlatform)
        {
            if (EOSString_ProductUserId::IsValid(Candidate.ProductUserId) &&
                *ConstCastSharedRef<const FUniqueNetIdEOS>(MakeShared<FUniqueNetIdEOS>(Candidate.ProductUserId)) ==
                    *State.ExistingUserId)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    }

    return false;
}

static bool CrossPlatformEOSCandidateIsContinuanceToken(const FAuthenticationGraphState &State)
{
    for (const auto &Candidate : State.EOSCandidates)
    {
        if (Candidate.Type == EAuthenticationGraphEOSCandidateType::CrossPlatform)
        {
            if (!EOSString_ContinuanceToken::IsNone(Candidate.ContinuanceToken))
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    }

    return false;
}

TSharedRef<FAuthenticationGraphNode> FEpicGamesCrossPlatformAccountProvider::GetInteractiveAuthenticationSequence(
    bool bAllowPersistentCredentials) const
{
    auto AuthSequence = MakeShared<FAuthenticationGraphNodeUntil_CrossPlatformAccountPresent>(
                            TEXT("Unable to sign in to Epic Games interactively. Check the logs for more information."))
                            ->Add(MakeShared<FTryExchangeCodeAuthenticationNode>())
                            ->Add(MakeShared<FTryPIEDeveloperAuthenticationEASCredentialsNode>())
                            ->Add(MakeShared<FTryDefaultDeveloperAuthenticationEASCredentialsNode>());

    if (bAllowPersistentCredentials)
    {
        // @note: Only try persistent credentials if bAllowPersistentCredentials is true
        AuthSequence->Add(MakeShared<FTryPersistentEASCredentialsNode>());
    }

    AuthSequence->Add(MakeShared<FGatherEASAccountsWithExternalCredentialsNode>())
        ->Add(MakeShared<FAuthenticationGraphNodeConditional>()
                  ->If(
                      FAuthenticationGraphCondition::CreateStatic(&HasContinuanceToken),
                      MakeShared<FPerformInteractiveLinkExternalCredentialsToEASAccountNode>())
                  ->Else(MakeShared<FPerformInteractiveEASLoginNode>()));

    auto FlowSequence = MakeShared<FAuthenticationGraphNodeUntil_Forever>();

    if (bAllowPersistentCredentials)
    {
        // Force deletion of the persistent auth token whenever the user signs out if we use the persistent auth
        // node. This ensures that when a developer is using "Cross Platform Only", we always delete persistent auth
        // token on logout so that the user can go through an interactive login again, regardless of how they signed
        // into an Epic Games account.
        //
        // We don't do this node if the "always persistent auth" node is not used. External accounts can still
        // request persistent auth to be done for authentication graphs other than "Cross Platform Only", but in
        // those cases we don't actually attempt persistent auth unless the external account matches and thus
        // persistent auth can't have an impact on logout/login scenarios because the user identity is tied to the
        // platform account, not Epic Games.
        FlowSequence->Add(MakeShared<FDeletePersistentAuthOnLogoutNode>());
    }

    return FlowSequence->Add(AuthSequence)->Add(MakeShared<FChainEASResultToEOSNode>());
}

TSharedRef<FAuthenticationGraphNode> FEpicGamesCrossPlatformAccountProvider::GetInteractiveOnlyAuthenticationSequence()
    const
{
    return MakeShared<FAuthenticationGraphNodeUntil_Forever>()
        ->Add(MakeShared<FAuthenticationGraphNodeUntil_CrossPlatformAccountPresent>()->Add(
            MakeShared<FPerformInteractiveEASLoginNode>()))
        ->Add(MakeShared<FChainEASResultToEOSNode>());
}

TSharedRef<FAuthenticationGraphNode> FEpicGamesCrossPlatformAccountProvider::GetNonInteractiveAuthenticationSequence(
    bool bOnlyUseExternalCredentials) const
{
    auto Sequence = MakeShared<FAuthenticationGraphNodeUntil_CrossPlatformAccountPresent>()->AllowFailure(true);
    if (!bOnlyUseExternalCredentials)
    {
        Sequence->Add(MakeShared<FTryExchangeCodeAuthenticationNode>())
            ->Add(MakeShared<FTryPIEDeveloperAuthenticationEASCredentialsNode>())
            ->Add(MakeShared<FTryDefaultDeveloperAuthenticationEASCredentialsNode>());
    }
    Sequence->Add(MakeShared<FGatherEASAccountsWithExternalCredentialsNode>());
    return MakeShared<FAuthenticationGraphNodeUntil_Forever>()->Add(Sequence)->Add(
        MakeShared<FAuthenticationGraphNodeConditional>()
            ->If(
                FAuthenticationGraphCondition::CreateStatic(
                    &FAuthenticationGraph::Condition_CrossPlatformAccountIsValid),
                MakeShared<FChainEASResultToEOSNode>())
            ->Else(MakeShared<FNoopAuthenticationGraphNode>()));
}

TSharedRef<FAuthenticationGraphNode> FEpicGamesCrossPlatformAccountProvider::
    GetUpgradeCurrentAccountToCrossPlatformAccountSequence() const
{
    return MakeShared<FAuthenticationGraphNodeUntil_Forever>()
        ->Add(MakeShared<FGetEASContinuanceTokenOrAccountForExistingExternalCredentialsNode>())
        //
        // We will now either have:
        // * State->EASExternalContinuanceToken if the external credentials are not associated with an Epic account
        //   (which is what we expect in 99% of cases here; the exception is if the user has linked the platform
        //    with an Epic Games account *while* the game has been running)
        // * State->AuthenticatedCrossPlatformAccountId if the external credentials are associated with an Epic
        //   account.
        //
        // Regardless of which one we have, we'll eventually have to check if we can use
        // State->AuthenticatedCrossPlatformAccountId. It can only be used if it ends up resolving to either
        // no EOS account, or the same EOS account as the current user is signed in. Otherwise, the Epic account
        // points to a different user profile.
        //
        // Before we do any of that, we have to convert State->EASExternalContinuanceToken to
        // State->AuthenticatedCrossPlatformAccountId by going through EOS_Auth_LinkAccount.
        //
        ->Add(MakeShared<FAuthenticationGraphNodeConditional>()
                  ->If(
                      FAuthenticationGraphCondition::CreateStatic(&HasContinuanceToken),
                      MakeShared<FPerformInteractiveLinkExternalCredentialsToEASAccountNode>())
                  ->Else(MakeShared<FNoopAuthenticationGraphNode>()))
        // We should now have State->AuthenticatedCrossPlatformAccountId.
        ->Add(MakeShared<FAuthenticationGraphNodeConditional>()
                  ->If(
                      FAuthenticationGraphCondition::CreateStatic(&HasEpicGamesAccount),
                      MakeShared<FChainEASResultToEOSNode>())
                  ->Else(MakeShared<FFailAuthenticationNode>(
                      "Unable to complete linking process as no Epic Games account was available.")))
        // We'll now have an EOS candidate with a CrossPlatform type.
        ->Add(MakeShared<FAuthenticationGraphNodeConditional>()
                  ->If(
                      FAuthenticationGraphCondition::CreateStatic(&CrossPlatformEOSCandidateMatchesExistingAccount),
                      MakeShared<FAuthenticationGraphNodeUntil_Forever>()
                          ->Add(MakeShared<FSelectCrossPlatformAccountNode>())
                          ->Add(MakeShared<FLoginWithSelectedEOSAccountNode>()))
                  ->If(
                      FAuthenticationGraphCondition::CreateStatic(&CrossPlatformEOSCandidateIsContinuanceToken),
                      MakeShared<FAuthenticationGraphNodeUntil_Forever>()
                          ->Add(MakeShared<FLinkEASContinuanceTokenToExistingAccountNode>())
                          // Then re-authenticate with the external credentials so we get a full Epic login done.
                          ->Add(MakeShared<FClearEOSCandidatesNode>())
                          ->Add(MakeShared<FGetEASContinuanceTokenOrAccountForExistingExternalCredentialsNode>())
                          ->Add(MakeShared<FChainEASResultToEOSNode>())
                          ->Add(MakeShared<FSelectCrossPlatformAccountNode>())
                          ->Add(MakeShared<FLoginWithSelectedEOSAccountNode>()))
                  ->Else(MakeShared<FFailAuthenticationDueToConflictingAccountsNode>()));
}

TSharedRef<FAuthenticationGraphNode> FEpicGamesCrossPlatformAccountProvider::
    GetLinkUnusedExternalCredentialsToCrossPlatformAccountSequence() const
{
    return MakeShared<FLinkUnconnectedEOSAccountToSignedInEASAccountNode>();
}

TSharedRef<FAuthenticationGraphNode> FEpicGamesCrossPlatformAccountProvider::GetNonInteractiveDeauthenticationSequence()
    const
{
    return MakeShared<FSignOutEASCandidateNode>();
}

TSharedRef<FAuthenticationGraphNode> FEpicGamesCrossPlatformAccountProvider::GetAutomatedTestingAuthenticationSequence()
    const
{
#if !defined(UE_BUILD_SHIPPING) || !UE_BUILD_SHIPPING
    return MakeShared<FAuthenticationGraphNodeUntil_Forever>()
        ->Add(MakeShared<FAuthenticationGraphNodeUntil_CrossPlatformAccountPresent>()->Add(
            MakeShared<FPerformAutomatedTestingEASLoginNode>()))
        ->Add(MakeShared<FChainEASResultToEOSNode>());
#else
    return MakeShared<FFailAuthenticationNode>(
        TEXT("Automated testing authentication is not supported in shipping builds."));
#endif
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()