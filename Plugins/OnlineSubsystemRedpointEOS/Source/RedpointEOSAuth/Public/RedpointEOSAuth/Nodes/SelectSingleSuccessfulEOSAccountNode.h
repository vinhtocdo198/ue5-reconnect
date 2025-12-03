// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAuth/AuthenticationGraph.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3191925516, Redpoint::EOS::Auth::Nodes)
{
using namespace ::Redpoint::EOS::Auth;

class REDPOINTEOSAUTH_API FSelectSingleSuccessfulEOSAccountNode : public FAuthenticationGraphNode
{
public:
    UE_NONCOPYABLE(FSelectSingleSuccessfulEOSAccountNode);
    FSelectSingleSuccessfulEOSAccountNode() = default;
    virtual ~FSelectSingleSuccessfulEOSAccountNode() override = default;

    virtual void Execute(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone) override;

    virtual FString GetDebugName() const override
    {
        return TEXT("FSelectSingleSuccessfulEOSAccountNode");
    }
};

}

namespace Redpoint::EOS::Auth::Nodes
{
REDPOINT_EOS_FILE_NS_EXPORT(3191925516, Redpoint::EOS::Auth::Nodes, FSelectSingleSuccessfulEOSAccountNode)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()