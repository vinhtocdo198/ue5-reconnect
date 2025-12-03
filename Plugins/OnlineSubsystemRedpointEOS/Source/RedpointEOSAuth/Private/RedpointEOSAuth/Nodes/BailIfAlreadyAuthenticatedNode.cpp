// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/Nodes/BailIfAlreadyAuthenticatedNode.h"

#include "RedpointEOSAuth/AuthenticationGraphState.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2422983380, Redpoint::EOS::Auth::Nodes)
{

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

void FBailIfAlreadyAuthenticatedNode::Execute(
    TSharedRef<FAuthenticationGraphState> State,
    FAuthenticationGraphNodeOnDone OnDone)
{
    if (State->ExistingUserId.IsValid())
    {
        State->ErrorMessages.Add(TEXT("User is already authenticated"));
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
    }
    else
    {
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
    }
}

#endif

}

REDPOINT_EOS_CODE_GUARD_END()