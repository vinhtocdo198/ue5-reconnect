// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAuth/AuthenticationGraph.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(77089298, Redpoint::EOS::Auth::Nodes)
{
using namespace ::Redpoint::EOS::Auth;

class REDPOINTEOSAUTH_API FTryDeviceIdAuthenticationNode : public FAuthenticationGraphNode
{
public:
    FTryDeviceIdAuthenticationNode() = default;
    UE_NONCOPYABLE(FTryDeviceIdAuthenticationNode);
    virtual ~FTryDeviceIdAuthenticationNode() override = default;

    virtual void Execute(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone) override;

    virtual FString GetDebugName() const override
    {
        return TEXT("FTryDeviceIdAuthenticationNode");
    }
};

}

namespace Redpoint::EOS::Auth::Nodes
{
REDPOINT_EOS_FILE_NS_EXPORT(77089298, Redpoint::EOS::Auth::Nodes, FTryDeviceIdAuthenticationNode)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()