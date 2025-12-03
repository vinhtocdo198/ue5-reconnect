// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAuth/AuthenticationGraphNodeOnDone.h"
#include "RedpointEOSAuth/ForwardDecls.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(2084317866, Redpoint::EOS::Auth)
{
using namespace ::Redpoint::EOS::Auth;

class REDPOINTEOSAUTH_API FAuthenticationGraphNode : public TSharedFromThis<FAuthenticationGraphNode>
{
public:
    FAuthenticationGraphNode() = default;
    UE_NONCOPYABLE(FAuthenticationGraphNode);
    virtual ~FAuthenticationGraphNode() = default;

    virtual void Execute(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone) = 0;
    virtual FString GetDebugName() const = 0;

    virtual void CollectChildNodes(FAuthenticationGraphNodeCollector &Collector);
};

}

namespace Redpoint::EOS::Auth
{
REDPOINT_EOS_FILE_NS_EXPORT(2084317866, Redpoint::EOS::Auth, FAuthenticationGraphNode)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()