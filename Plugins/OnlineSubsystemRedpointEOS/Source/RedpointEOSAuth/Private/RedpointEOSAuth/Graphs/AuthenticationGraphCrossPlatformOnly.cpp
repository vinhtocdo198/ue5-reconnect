// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/Graphs/AuthenticationGraphCrossPlatformOnly.h"

#include "RedpointEOSAuth/AuthenticationGraphRegistry.h"

#include "RedpointEOSAuth/Nodes/AuthenticationGraphNodeUntil_Forever.h"
#include "RedpointEOSAuth/Nodes/BailIfAlreadyAuthenticatedNode.h"
#include "RedpointEOSAuth/Nodes/FailAuthenticationNode.h"
#include "RedpointEOSAuth/Nodes/LoginWithSelectedEOSAccountNode.h"
#include "RedpointEOSAuth/Nodes/SelectCrossPlatformAccountNode.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2028868234, Redpoint::EOS::Auth::Graphs)
{
using namespace ::Redpoint::EOS::Auth;
using namespace ::Redpoint::EOS::Auth::Nodes;

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

TSharedRef<FAuthenticationGraphNode> FAuthenticationGraphCrossPlatformOnly::CreateGraph(
    const TSharedRef<FAuthenticationGraphState> &InitialState) const
{
    if (InitialState->CrossPlatformAccountProvider.IsValid())
    {
        return MakeShared<FAuthenticationGraphNodeUntil_Forever>()
            ->Add(MakeShared<FBailIfAlreadyAuthenticatedNode>())
            ->Add(InitialState->CrossPlatformAccountProvider->GetInteractiveAuthenticationSequence(true))
            ->Add(MakeShared<FSelectCrossPlatformAccountNode>())
            ->Add(MakeShared<FLoginWithSelectedEOSAccountNode>());
    }
    else
    {
        return MakeShared<FFailAuthenticationNode>(TEXT("There is no cross-platform account provider configured."));
    }
}

void FAuthenticationGraphCrossPlatformOnly::Register()
{
    FAuthenticationGraphRegistry::Register(
        EOS_AUTH_GRAPH_CROSS_PLATFORM_ONLY,
        NSLOCTEXT("OnlineSubsystemRedpointEOS", "AuthGraph_CrossPlatformOnly", "Cross-Platform Only"),
        MakeShared<FAuthenticationGraphCrossPlatformOnly>());
}

#endif

}

REDPOINT_EOS_CODE_GUARD_END()