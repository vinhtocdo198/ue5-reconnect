// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAuth/AuthenticationGraph.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1791657458, Redpoint::EOS::Auth::Nodes)
{
using namespace ::Redpoint::EOS::Auth;

class REDPOINTEOSAUTH_API FBailIfNotExactlyOneExternalCredentialNode : public FAuthenticationGraphNode
{
public:
    UE_NONCOPYABLE(FBailIfNotExactlyOneExternalCredentialNode);
    FBailIfNotExactlyOneExternalCredentialNode() = default;
    virtual ~FBailIfNotExactlyOneExternalCredentialNode() override = default;

    virtual void Execute(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone) override;

    virtual FString GetDebugName() const override
    {
        return TEXT("FBailIfNotExactlyOneExternalCredentialNode");
    }
};

}

namespace Redpoint::EOS::Auth::Nodes
{
REDPOINT_EOS_FILE_NS_EXPORT(1791657458, Redpoint::EOS::Auth::Nodes, FBailIfNotExactlyOneExternalCredentialNode)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()