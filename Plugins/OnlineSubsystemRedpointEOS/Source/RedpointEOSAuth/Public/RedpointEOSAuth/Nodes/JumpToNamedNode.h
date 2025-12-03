// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAuth/AuthenticationGraph.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1809599676, Redpoint::EOS::Auth::Nodes)
{
using namespace ::Redpoint::EOS::Auth;

class REDPOINTEOSAUTH_API FJumpToNamedNode : public FAuthenticationGraphNode
{
private:
    TSharedRef<FAuthenticationGraph> Graph;
    FName TargetName;

    void HandleOnDone(
        EAuthenticationGraphNodeResult Result,
        TSharedPtr<FAuthenticationGraphNode> Node,
        TSharedRef<FAuthenticationGraphState> State,
        FAuthenticationGraphNodeOnDone OnDone);

public:
    UE_NONCOPYABLE(FJumpToNamedNode);
    virtual ~FJumpToNamedNode() override = default;

    FJumpToNamedNode(TSharedRef<FAuthenticationGraph> InGraph, FName InTargetName)
        : Graph(MoveTemp(InGraph))
        , TargetName(InTargetName){};

    virtual void Execute(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone) override;

    virtual FString GetDebugName() const override
    {
        return TEXT("FJumpToNamedNode");
    }
};

}

namespace Redpoint::EOS::Auth::Nodes
{
REDPOINT_EOS_FILE_NS_EXPORT(1809599676, Redpoint::EOS::Auth::Nodes, FJumpToNamedNode)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()