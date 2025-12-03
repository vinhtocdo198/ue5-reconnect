// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/AuthenticationGraphNode.h"

#include "RedpointEOSAuth/AuthenticationGraph.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2084317866, Redpoint::EOS::Auth)
{

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

void FAuthenticationGraphNode::CollectChildNodes(FAuthenticationGraphNodeCollector &Collector)
{
}

#endif

}

REDPOINT_EOS_CODE_GUARD_END()