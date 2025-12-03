// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/Nodes/SelectOnlyEOSCandidateNode.h"

#include "RedpointEOSAuth/AuthenticationGraphState.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2192422034, Redpoint::EOS::Auth::Nodes)
{

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

void FSelectOnlyEOSCandidateNode::Execute(
    TSharedRef<FAuthenticationGraphState> State,
    FAuthenticationGraphNodeOnDone OnDone)
{
    // We can have no EOS candidates if authentication failed.
    if (State->EOSCandidates.Num() != 1)
    {
        State->ErrorMessages.Add(TEXT("Unable to successfully complete login."));
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
        return;
    }

    State->SelectEOSCandidate(State->EOSCandidates[0]);
    OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
}

#endif

}

REDPOINT_EOS_CODE_GUARD_END()