// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/AuthenticationGraphNodeCollector.h"

#include "RedpointEOSAuth/AuthenticationGraph.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1495801801, Redpoint::EOS::Auth)
{
using namespace ::Redpoint::EOS::Auth;

void FAuthenticationGraphNodeCollector::AddNode(const TSharedRef<FAuthenticationGraphNode> &InNode)
{
    if (!this->CollectedNodesValue.Contains(InNode))
    {
        this->CollectedNodesValue.Add(InNode);
        InNode->CollectChildNodes(*this);
    }
}

const TSet<TSharedRef<FAuthenticationGraphNode>> &FAuthenticationGraphNodeCollector::GetCollectedNodes() const
{
    return this->CollectedNodesValue;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()