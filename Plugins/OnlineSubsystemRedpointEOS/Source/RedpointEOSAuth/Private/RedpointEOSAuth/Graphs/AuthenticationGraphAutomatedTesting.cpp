// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/Graphs/AuthenticationGraphAutomatedTesting.h"

#include "RedpointEOSAuth/AuthenticationGraph.h"
#include "RedpointEOSAuth/AuthenticationGraphRegistry.h"
#include "RedpointEOSAuth/Graphs/AuthenticationGraphRuntimePlatform.h"
#include "RedpointEOSAuth/Nodes/AuthenticationGraphNodeUntil_Forever.h"
#include "RedpointEOSAuth/Nodes/AuthenticationGraphNodeUntil_LoginComplete.h"
#include "RedpointEOSAuth/Nodes/BailIfAlreadyAuthenticatedNode.h"
#include "RedpointEOSAuth/Nodes/IssueJwtForAutomatedTestingNode.h"
#include "RedpointEOSAuth/Nodes/LoginWithSelectedEOSAccountNode.h"
#include "RedpointEOSAuth/Nodes/PerformOpenIdLoginForAutomatedTestingNode.h"
#include "RedpointEOSAuth/Nodes/SelectAutomatedTestingEOSAccountNode.h"
#include "RedpointEOSAuth/Nodes/SelectCrossPlatformAccountNode.h"
#include "RedpointEOSAuth/Nodes/SelectSingleSuccessfulEOSAccountNode.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && (!defined(UE_BUILD_SHIPPING) || !UE_BUILD_SHIPPING)

namespace REDPOINT_EOS_FILE_NS_ID(2581304592, Redpoint::EOS::Auth::Graphs)
{
using namespace ::Redpoint::EOS::Auth;
using namespace ::Redpoint::EOS::Auth::Nodes;
using namespace ::Redpoint::EOS::Auth::Graphs;

TSharedRef<FAuthenticationGraphNode> FAuthenticationGraphAutomatedTesting::CreateGraph(
    const TSharedRef<FAuthenticationGraphState> &InitialState) const
{
    if (InitialState->AutomatedTestingEmailAddress.StartsWith(TEXT("CreateOnDemand:")))
    {
        return MakeShared<FAuthenticationGraphNodeUntil_Forever>()
            ->Add(MakeShared<FBailIfAlreadyAuthenticatedNode>())
            ->Add(MakeShared<FIssueJwtForAutomatedTestingNode>())
            ->Add(MakeShared<FPerformOpenIdLoginForAutomatedTestingNode>())
            ->Add(MakeShared<FSelectAutomatedTestingEOSAccountNode>())
            ->Add(MakeShared<FLoginWithSelectedEOSAccountNode>());
    }
    else
    {
        checkf(
            InitialState->CrossPlatformAccountProvider.IsValid(),
            TEXT("Cross-platform account provider isn't set, but automated testing node requires the Epic Games "
                 "cross-platform account provider."));

        return MakeShared<FAuthenticationGraphNodeUntil_Forever>()
            ->Add(MakeShared<FBailIfAlreadyAuthenticatedNode>())
            ->Add(MakeShared<FAuthenticationGraphNodeUntil_LoginComplete>()
                      ->Add(InitialState->CrossPlatformAccountProvider->GetAutomatedTestingAuthenticationSequence())
                      ->Add(MakeShared<FSelectCrossPlatformAccountNode>())
                      ->Add(MakeShared<FLoginWithSelectedEOSAccountNode>()));
    }
}

void FAuthenticationGraphAutomatedTesting::Register()
{
#if 0 // @todo Restore this functionality for auth graph tests
    FAuthenticationGraphOnlineSubsystem::RegisterForCustomPlatform(
        EOS_AUTH_GRAPH_AUTOMATED_TESTING_OSS,
        NSLOCTEXT(
            "OnlineSubsystemRedpointEOS",
            "AuthGraph_AutomatedTestingOSS",
            "Automated Testing (Online Subsystem)"),
        TEXT("NULL"),
        EOS_EExternalCredentialType::EOS_ECT_OPENID_ACCESS_TOKEN,
        TEXT("unused"),
        TEXT("unused"));
#endif
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()