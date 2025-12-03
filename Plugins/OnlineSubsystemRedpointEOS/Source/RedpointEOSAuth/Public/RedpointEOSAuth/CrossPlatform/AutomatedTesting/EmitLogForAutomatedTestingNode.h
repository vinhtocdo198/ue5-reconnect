// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAuth/AuthenticationGraph.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && (!defined(UE_BUILD_SHIPPING) || !UE_BUILD_SHIPPING)

namespace REDPOINT_EOS_FILE_NS_ID(1284975154, Redpoint::EOS::Auth::CrossPlatform::AutomatedTesting)
{
using namespace ::Redpoint::EOS::Auth;

class REDPOINTEOSAUTH_API FEmitLogForAutomatedTestingNode : public FAuthenticationGraphNode
{
private:
    FString Log;

public:
    UE_NONCOPYABLE(FEmitLogForAutomatedTestingNode);
    FEmitLogForAutomatedTestingNode(const FString &InLog);

    virtual void Execute(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone) override;

    virtual FString GetDebugName() const override
    {
        return TEXT("FEmitLogForAutomatedTestingNode");
    }
};

}

namespace Redpoint::EOS::Auth::CrossPlatform::AutomatedTesting
{
REDPOINT_EOS_FILE_NS_EXPORT(
    1284975154,
    Redpoint::EOS::Auth::CrossPlatform::AutomatedTesting,
    FEmitLogForAutomatedTestingNode)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()