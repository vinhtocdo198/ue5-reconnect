// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAuth/ForwardDecls.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1495801801, Redpoint::EOS::Auth)
{
using namespace ::Redpoint::EOS::Auth;

class REDPOINTEOSAUTH_API FAuthenticationGraphNodeCollector : public TSharedFromThis<FAuthenticationGraphNodeCollector>
{
private:
    TSet<TSharedRef<FAuthenticationGraphNode>> CollectedNodesValue;

public:
    FAuthenticationGraphNodeCollector() = default;
    UE_NONCOPYABLE(FAuthenticationGraphNodeCollector);
    virtual ~FAuthenticationGraphNodeCollector() = default;

    void AddNode(const TSharedRef<FAuthenticationGraphNode> &InNode);
    const TSet<TSharedRef<FAuthenticationGraphNode>> &GetCollectedNodes() const;
};

}

namespace Redpoint::EOS::Auth
{
REDPOINT_EOS_FILE_NS_EXPORT(1495801801, Redpoint::EOS::Auth, FAuthenticationGraphNodeCollector)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()