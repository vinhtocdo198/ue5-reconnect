// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAuth/AuthenticationGraph.h"

#include "Interfaces/OnlineIdentityInterface.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(320275565, Redpoint::EOS::Auth::Nodes)
{
using namespace ::Redpoint::EOS::Auth;

class REDPOINTEOSAUTH_API FCheckUserPrivilegesNode : public FAuthenticationGraphNode
{
public:
    FCheckUserPrivilegesNode() = default;
    UE_NONCOPYABLE(FCheckUserPrivilegesNode);
    virtual ~FCheckUserPrivilegesNode() override = default;

    virtual void Execute(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone) override;

    virtual FString GetDebugName() const override
    {
        return TEXT("FCheckUserPrivilegesNode");
    }
};

}

namespace Redpoint::EOS::Auth::Nodes
{
REDPOINT_EOS_FILE_NS_EXPORT(320275565, Redpoint::EOS::Auth::Nodes, FCheckUserPrivilegesNode)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()