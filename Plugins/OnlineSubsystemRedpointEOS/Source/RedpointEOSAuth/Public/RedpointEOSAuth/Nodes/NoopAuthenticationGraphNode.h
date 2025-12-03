// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAuth/AuthenticationGraphNode.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1572695714, Redpoint::EOS::Auth::Nodes)
{
using namespace ::Redpoint::EOS::Auth;

class REDPOINTEOSAUTH_API FNoopAuthenticationGraphNode : public FAuthenticationGraphNode
{
public:
    UE_NONCOPYABLE(FNoopAuthenticationGraphNode);
    FNoopAuthenticationGraphNode() = default;
    virtual ~FNoopAuthenticationGraphNode() override = default;

    virtual void Execute(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone) override;

    virtual FString GetDebugName() const override
    {
        return TEXT("FNoopAuthenticationGraphNode");
    }
};

}

namespace Redpoint::EOS::Auth::Nodes
{
REDPOINT_EOS_FILE_NS_EXPORT(1572695714, Redpoint::EOS::Auth::Nodes, FNoopAuthenticationGraphNode)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()