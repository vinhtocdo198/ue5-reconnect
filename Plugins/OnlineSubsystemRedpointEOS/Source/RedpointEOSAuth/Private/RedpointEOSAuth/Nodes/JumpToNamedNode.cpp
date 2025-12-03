// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/Nodes/JumpToNamedNode.h"

#include "RedpointEOSAuth/AuthenticationGraphState.h"
#include "RedpointEOSAuth/Logging.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1809599676, Redpoint::EOS::Auth::Nodes)
{

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

void FJumpToNamedNode::Execute(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone)
{
    TSharedPtr<FAuthenticationGraphNode> Node = State->RegisteredNodes.FindRef(this->TargetName);

    UE_LOG(
        LogRedpointEOSAuth,
        Verbose,
        TEXT("%p: Authentication graph entering node: %s"),
        &State.Get(),
        *Node->GetDebugName());
    Node->Execute(
        State,
        FAuthenticationGraphNodeOnDone::CreateSP(this, &FJumpToNamedNode::HandleOnDone, Node, State, OnDone));
}

void FJumpToNamedNode::HandleOnDone(
    EAuthenticationGraphNodeResult Result,
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    TSharedPtr<FAuthenticationGraphNode> Node,
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    TSharedRef<FAuthenticationGraphState> State,
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    FAuthenticationGraphNodeOnDone OnDone)
{
    UE_LOG(
        LogRedpointEOSAuth,
        Verbose,
        TEXT("%p: Authentication graph exiting node: %s"),
        &State.Get(),
        *Node->GetDebugName());

    OnDone.ExecuteIfBound(Result);
}

#endif

}

REDPOINT_EOS_CODE_GUARD_END()