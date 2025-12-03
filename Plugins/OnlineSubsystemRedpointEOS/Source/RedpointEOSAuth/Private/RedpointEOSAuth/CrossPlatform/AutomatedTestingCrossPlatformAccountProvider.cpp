// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/CrossPlatform/AutomatedTestingCrossPlatformAccountProvider.h"

#include "RedpointEOSAuth/AuthenticationGraph.h"
#include "RedpointEOSAuth/CrossPlatform/AutomatedTesting/EmitLogForAutomatedTestingNode.h"
#include "RedpointEOSAuth/CrossPlatform/AutomatedTesting/IssueJwtForCrossPlatformAutomatedTestingNode.h"
#include "RedpointEOSAuth/CrossPlatform/AutomatedTesting/PerformOpenIdLoginForCrossPlatformATNode.h"
#include "RedpointEOSAuth/CrossPlatform/AutomatedTestingCrossPlatformAccountId.h"
#include "RedpointEOSAuth/Nodes/AuthenticationGraphNodeUntil_Forever.h"
#include "RedpointEOSAuth/Nodes/FailAuthenticationNode.h"
#include "RedpointEOSAuth/Nodes/IssueJwtForAutomatedTestingNode.h"
#include "RedpointEOSAuth/Nodes/LoginWithSelectedEOSAccountNode.h"
#include "RedpointEOSAuth/Nodes/NoopAuthenticationGraphNode.h"
#include "RedpointEOSAuth/Nodes/PerformOpenIdLoginForAutomatedTestingNode.h"
#include "RedpointEOSAuth/Nodes/SelectCrossPlatformAccountNode.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && (!defined(UE_BUILD_SHIPPING) || !UE_BUILD_SHIPPING)

namespace REDPOINT_EOS_FILE_NS_ID(2701208506, Redpoint::EOS::Auth::CrossPlatform)
{
using namespace ::Redpoint::EOS::Auth::Nodes;
using namespace ::Redpoint::EOS::Auth::CrossPlatform::AutomatedTesting;

FName FAutomatedTestingCrossPlatformAccountProvider::GetName() const
{
    return AUTOMATED_TESTING_ACCOUNT_ID;
}

TSharedPtr<const FCrossPlatformAccountId> FAutomatedTestingCrossPlatformAccountProvider::CreateCrossPlatformAccountId(
    const FString &InStringRepresentation) const
{
    return FAutomatedTestingCrossPlatformAccountId::ParseFromString(InStringRepresentation);
}

TSharedPtr<const FCrossPlatformAccountId> FAutomatedTestingCrossPlatformAccountProvider::CreateCrossPlatformAccountId(
    uint8 *InBytes,
    int32 InSize) const
{
    FString Data = BytesToString(InBytes, InSize);
    return FAutomatedTestingCrossPlatformAccountId::ParseFromString(Data);
}

TSharedRef<FAuthenticationGraphNode> FAutomatedTestingCrossPlatformAccountProvider::
    GetInteractiveAuthenticationSequence(bool bAllowPersistentCredentials) const
{
    // This will fire if the game requires a cross-platform account.
    // It needs to emit a log entry for the unit test to detect, and then do an automated sign-in.
    return MakeShared<FAuthenticationGraphNodeUntil_Forever>()
        ->Add(MakeShared<FIssueJwtForCrossPlatformAutomatedTestingNode>())
        ->Add(MakeShared<FEmitLogForAutomatedTestingNode>(
            TEXT("[CPAT-01] Emulating required sign-in for cross-platform automation testing")))
        ->Add(MakeShared<FPerformOpenIdLoginForCrossPlatformAutomatedTestingNode>());
}

TSharedRef<FAuthenticationGraphNode> FAutomatedTestingCrossPlatformAccountProvider::
    GetInteractiveOnlyAuthenticationSequence() const
{
    // This will fire after the "user" picks "sign in" instead of "create an account" at the prompt.
    // It needs to emit a log entry for the unit test to detect, and then do an automated sign-in.
    return MakeShared<FAuthenticationGraphNodeUntil_Forever>()
        ->Add(MakeShared<FIssueJwtForCrossPlatformAutomatedTestingNode>())
        ->Add(MakeShared<FEmitLogForAutomatedTestingNode>(
            TEXT("[CPAT-02] Emulating interactive sign in because the automated testing emulated a click on 'sign in' "
                 "instead of 'create an account'")))
        ->Add(MakeShared<FPerformOpenIdLoginForCrossPlatformAutomatedTestingNode>());
}

TSharedRef<FAuthenticationGraphNode> FAutomatedTestingCrossPlatformAccountProvider::
    GetNonInteractiveAuthenticationSequence(bool bOnlyUseExternalCredentials) const
{
    // This fires during an optional sign in process, and would be used to try persistent logins, dev tools and
    // already gathered external credentials. Because JWTs are always new accounts, this will always result in an EOS
    // candidate that has a continuance token (not a PUID) and thus cause the "sign-in / create account" prompt to show.
    // We do a normal (not cross-platform) JWT issuance here so that when FSelectSingleContinuanceTokenEOSCandidateNode
    // runs later in the graph if "create account" is selected, it actually has a valid continuance token with which to
    // create an EOS account.
    return MakeShared<FAuthenticationGraphNodeUntil_Forever>()
        ->Add(MakeShared<FEmitLogForAutomatedTestingNode>(
            TEXT("[CPAT-03] Emulating non-interactive sign-in for cross-platform automation testing")))
        ->Add(MakeShared<FIssueJwtForAutomatedTestingNode>())
        ->Add(MakeShared<FPerformOpenIdLoginForAutomatedTestingNode>());
}

TSharedRef<FAuthenticationGraphNode> FAutomatedTestingCrossPlatformAccountProvider::
    GetUpgradeCurrentAccountToCrossPlatformAccountSequence() const
{
    // This will fire when the user is already signed into a platform-specific account and they want to upgrade to a
    // cross-platform account. Since this graph must complete with an authenticated cross-platform account, we just do a
    // normal login here and emit a different log code for the unit tests.
    return MakeShared<FAuthenticationGraphNodeUntil_Forever>()
        ->Add(MakeShared<FIssueJwtForCrossPlatformAutomatedTestingNode>())
        ->Add(MakeShared<FEmitLogForAutomatedTestingNode>(
            TEXT("[CPAT-05] Emulating interactive login as part of an upgrade process")))
        ->Add(MakeShared<FPerformOpenIdLoginForCrossPlatformAutomatedTestingNode>())
        ->Add(MakeShared<FSelectCrossPlatformAccountNode>())
        ->Add(MakeShared<FLoginWithSelectedEOSAccountNode>());
}

TSharedRef<FAuthenticationGraphNode> FAutomatedTestingCrossPlatformAccountProvider::
    GetLinkUnusedExternalCredentialsToCrossPlatformAccountSequence() const
{
    // This sequence of nodes must always succeed, but is otherwise unused.
    return MakeShared<FEmitLogForAutomatedTestingNode>(
        TEXT("[CPAT-04] Requested linkage of platform credentials into cross-platform account"));
}

TSharedRef<FAuthenticationGraphNode> FAutomatedTestingCrossPlatformAccountProvider::
    GetNonInteractiveDeauthenticationSequence() const
{
    return MakeShared<FNoopAuthenticationGraphNode>();
}

TSharedRef<FAuthenticationGraphNode> FAutomatedTestingCrossPlatformAccountProvider::
    GetAutomatedTestingAuthenticationSequence() const
{
    return MakeShared<FFailAuthenticationNode>(
        TEXT("The automated testing cross-platform provider is not meant to be used with the AutomatedTesting graph, "
             "as this combination is redundant. The automated testing cross-platform provider is intended to be used "
             "with other graphs to test their interactive sign in and create account flows."));
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()