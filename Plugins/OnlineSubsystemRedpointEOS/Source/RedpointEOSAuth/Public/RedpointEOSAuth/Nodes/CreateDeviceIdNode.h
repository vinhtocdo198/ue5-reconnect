// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAuth/AuthenticationGraph.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1135899555, Redpoint::EOS::Auth::Nodes)
{
using namespace ::Redpoint::EOS::Auth;

class REDPOINTEOSAUTH_API FCreateDeviceIdNode : public FAuthenticationGraphNode
{
public:
    UE_NONCOPYABLE(FCreateDeviceIdNode);
    FCreateDeviceIdNode() = default;
    virtual ~FCreateDeviceIdNode() override = default;

    virtual void Execute(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone) override;

    virtual FString GetDebugName() const override
    {
        return TEXT("FCreateDeviceIdNode");
    }
};

}

namespace Redpoint::EOS::Auth::Nodes
{
REDPOINT_EOS_FILE_NS_EXPORT(1135899555, Redpoint::EOS::Auth::Nodes, FCreateDeviceIdNode)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()