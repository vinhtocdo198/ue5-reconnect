// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSAuth/AuthenticationGraph.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3268046824, Redpoint::EOS::Auth::Nodes)
{
using namespace ::Redpoint::EOS::Auth;

class REDPOINTEOSAUTH_API FSignOutEOSAccountNode : public FAuthenticationGraphNode
{
private:
    EOS_ProductUserId LocalUserId;

public:
    UE_NONCOPYABLE(FSignOutEOSAccountNode);
    FSignOutEOSAccountNode(EOS_ProductUserId InLocalUserId);
    virtual ~FSignOutEOSAccountNode() override = default;

    virtual void Execute(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone) override;

    virtual FString GetDebugName() const override
    {
        return TEXT("FSignOutEOSAccountNode");
    }
};

}

namespace Redpoint::EOS::Auth::Nodes
{
REDPOINT_EOS_FILE_NS_EXPORT(3268046824, Redpoint::EOS::Auth::Nodes, FSignOutEOSAccountNode)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()