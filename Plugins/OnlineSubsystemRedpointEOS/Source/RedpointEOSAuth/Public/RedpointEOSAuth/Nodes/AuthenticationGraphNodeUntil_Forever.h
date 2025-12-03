// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAuth/Nodes/AuthenticationGraphNodeUntil.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(2668518940, Redpoint::EOS::Auth::Nodes)
{
using namespace ::Redpoint::EOS::Auth;
using namespace ::Redpoint::EOS::Auth::Nodes;

class REDPOINTEOSAUTH_API FAuthenticationGraphNodeUntil_Forever : public FAuthenticationGraphNodeUntil
{
private:
    static bool Condition(const FAuthenticationGraphState &State);
    virtual bool RequireConditionPass() const override;

public:
    FAuthenticationGraphNodeUntil_Forever();
    UE_NONCOPYABLE(FAuthenticationGraphNodeUntil_Forever);
    virtual ~FAuthenticationGraphNodeUntil_Forever() override = default;

    virtual FString GetDebugName() const override;
};

}

namespace Redpoint::EOS::Auth::Nodes
{
REDPOINT_EOS_FILE_NS_EXPORT(2668518940, Redpoint::EOS::Auth::Nodes, FAuthenticationGraphNodeUntil_Forever)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()