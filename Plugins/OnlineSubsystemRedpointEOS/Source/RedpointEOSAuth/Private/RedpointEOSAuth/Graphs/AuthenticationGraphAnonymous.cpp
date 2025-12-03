// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/Graphs/AuthenticationGraphAnonymous.h"

#include "RedpointEOSAuth/AuthenticationGraphRegistry.h"
#include "RedpointEOSAuth/Nodes/AuthenticationGraphNodeConditional.h"
#include "RedpointEOSAuth/Nodes/AuthenticationGraphNodeUntil_Forever.h"
#include "RedpointEOSAuth/Nodes/AuthenticationGraphNodeUntil_LoginComplete.h"
#include "RedpointEOSAuth/Nodes/BailIfAlreadyAuthenticatedNode.h"
#include "RedpointEOSAuth/Nodes/CreateDeviceIdNode.h"
#include "RedpointEOSAuth/Nodes/LoginWithSelectedEOSAccountNode.h"
#include "RedpointEOSAuth/Nodes/SelectOnlyEOSCandidateNode.h"
#include "RedpointEOSAuth/Nodes/TryDeviceIdAuthenticationNode.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3200112234, Redpoint::EOS::Auth::Graphs)
{
using namespace ::Redpoint::EOS::Auth;
using namespace ::Redpoint::EOS::Auth::Nodes;

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

static bool AnyCandidates(const FAuthenticationGraphState &State)
{
    return State.EOSCandidates.Num() > 0;
}

TSharedRef<FAuthenticationGraphNode> FAuthenticationGraphAnonymous::CreateGraph(
    const TSharedRef<FAuthenticationGraphState> &InitialState) const
{
    return MakeShared<FAuthenticationGraphNodeUntil_Forever>()
        ->Add(MakeShared<FBailIfAlreadyAuthenticatedNode>())
        ->Add(MakeShared<FTryDeviceIdAuthenticationNode>())
        ->Add(MakeShared<FAuthenticationGraphNodeConditional>()
                  ->If(
                      FAuthenticationGraphCondition::CreateStatic(&AnyCandidates),
                      MakeShared<FAuthenticationGraphNodeUntil_LoginComplete>()
                          ->Add(MakeShared<FSelectOnlyEOSCandidateNode>())
                          ->Add(MakeShared<FLoginWithSelectedEOSAccountNode>()))
                  ->Else(MakeShared<FAuthenticationGraphNodeUntil_LoginComplete>()
                             ->Add(MakeShared<FCreateDeviceIdNode>())
                             ->Add(MakeShared<FTryDeviceIdAuthenticationNode>())
                             ->Add(MakeShared<FSelectOnlyEOSCandidateNode>())
                             ->Add(MakeShared<FLoginWithSelectedEOSAccountNode>())));
}

void FAuthenticationGraphAnonymous::Register()
{
    FAuthenticationGraphRegistry::Register(
        EOS_AUTH_GRAPH_ANONYMOUS,
        NSLOCTEXT("OnlineSubsystemRedpointEOS", "AuthGraph_Anonymous", "Anonymous"),
        MakeShared<FAuthenticationGraphAnonymous>());
}

#endif

}

REDPOINT_EOS_CODE_GUARD_END()