// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAuth/AuthenticationGraph.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && (!defined(UE_BUILD_SHIPPING) || !UE_BUILD_SHIPPING)

namespace REDPOINT_EOS_FILE_NS_ID(3077486437, Redpoint::EOS::Auth::Nodes)
{
using namespace ::Redpoint::EOS::Auth;

class REDPOINTEOSAUTH_API FSelectAutomatedTestingEOSAccountNode : public FAuthenticationGraphNode
{
public:
    UE_NONCOPYABLE(FSelectAutomatedTestingEOSAccountNode);
    FSelectAutomatedTestingEOSAccountNode() = default;
    virtual ~FSelectAutomatedTestingEOSAccountNode() override = default;

    virtual void Execute(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone) override;

    virtual FString GetDebugName() const override
    {
        return TEXT("FSelectAutomatedTestingEOSAccountNode");
    }
};

}

namespace Redpoint::EOS::Auth::Nodes
{
REDPOINT_EOS_FILE_NS_EXPORT(3077486437, Redpoint::EOS::Auth::Nodes, FSelectAutomatedTestingEOSAccountNode)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()