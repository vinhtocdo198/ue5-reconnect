// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/Nodes/NoopAuthenticationGraphNode.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1572695714, Redpoint::EOS::Auth::Nodes)
{
using namespace ::Redpoint::EOS::Auth;

void FNoopAuthenticationGraphNode::Execute(
    TSharedRef<FAuthenticationGraphState> State,
    FAuthenticationGraphNodeOnDone OnDone)
{
    OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()