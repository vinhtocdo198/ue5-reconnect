// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAuth/AuthenticationGraph.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && (!defined(UE_BUILD_SHIPPING) || !UE_BUILD_SHIPPING)

namespace REDPOINT_EOS_FILE_NS_ID(968583338, Redpoint::EOS::Auth::Nodes)
{
using namespace ::Redpoint::EOS::Auth;

class REDPOINTEOSAUTH_API FPerformOpenIdLoginForAutomatedTestingNode : public FAuthenticationGraphNode
{
public:
    UE_NONCOPYABLE(FPerformOpenIdLoginForAutomatedTestingNode);
    FPerformOpenIdLoginForAutomatedTestingNode() = default;
    virtual ~FPerformOpenIdLoginForAutomatedTestingNode() override = default;

    virtual void Execute(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone) override;

    virtual FString GetDebugName() const override
    {
        return TEXT("FPerformOpenIdLoginForAutomatedTestingNode");
    }
};

}

namespace Redpoint::EOS::Auth::Nodes
{
REDPOINT_EOS_FILE_NS_EXPORT(968583338, Redpoint::EOS::Auth::Nodes, FPerformOpenIdLoginForAutomatedTestingNode)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()