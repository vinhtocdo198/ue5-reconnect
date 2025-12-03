// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/Nodes/FailAuthenticationNode.h"

#include "RedpointEOSAuth/AuthenticationGraphState.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2664489014, Redpoint::EOS::Auth::Nodes)
{

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

void FFailAuthenticationNode::Execute(
    TSharedRef<FAuthenticationGraphState> State,
    FAuthenticationGraphNodeOnDone OnDone)
{
    State->ErrorMessages.Add(this->ErrorMessage);
    OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
}

#endif

}

REDPOINT_EOS_CODE_GUARD_END()