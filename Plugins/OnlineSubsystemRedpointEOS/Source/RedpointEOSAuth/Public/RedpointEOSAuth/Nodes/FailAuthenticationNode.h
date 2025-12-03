// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAuth/AuthenticationGraph.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(2664489014, Redpoint::EOS::Auth::Nodes)
{
using namespace ::Redpoint::EOS::Auth;

class REDPOINTEOSAUTH_API FFailAuthenticationNode : public FAuthenticationGraphNode
{
private:
    FString ErrorMessage;

public:
    UE_NONCOPYABLE(FFailAuthenticationNode);
    FFailAuthenticationNode(FString InErrorMessage)
        : ErrorMessage(MoveTemp(InErrorMessage)){};
    virtual ~FFailAuthenticationNode() override = default;

    virtual void Execute(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone) override;

    virtual FString GetDebugName() const override
    {
        return TEXT("FFailAuthenticationNode");
    }
};

}

namespace Redpoint::EOS::Auth::Nodes
{
REDPOINT_EOS_FILE_NS_EXPORT(2664489014, Redpoint::EOS::Auth::Nodes, FFailAuthenticationNode)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()