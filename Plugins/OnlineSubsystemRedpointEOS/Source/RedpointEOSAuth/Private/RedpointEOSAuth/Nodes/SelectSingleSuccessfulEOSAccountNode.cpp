// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/Nodes/SelectSingleSuccessfulEOSAccountNode.h"

#include "RedpointEOSAuth/AuthenticationGraphState.h"
#include "RedpointEOSCore/Legacy/EOSString.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3191925516, Redpoint::EOS::Auth::Nodes)
{

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

void FSelectSingleSuccessfulEOSAccountNode::Execute(
    TSharedRef<FAuthenticationGraphState> State,
    FAuthenticationGraphNodeOnDone OnDone)
{
    for (const auto &Candidate : State->EOSCandidates)
    {
        if (EOSString_ProductUserId::IsValid(Candidate.ProductUserId))
        {
            if (State->HasSelectedEOSCandidate())
            {
                State->ErrorMessages.Add(
                    TEXT("SelectSingleSuccessfulEOSAccountNode hit with multiple valid candidates"));
                OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
                return;
            }
            State->SelectEOSCandidate(Candidate);
        }
    }

    if (!State->HasSelectedEOSCandidate())
    {
        State->ErrorMessages.Add(TEXT("SelectSingleSuccessfulEOSAccountNode hit with no valid candidates"));
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
        return;
    }

    OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
}

#endif

}

REDPOINT_EOS_CODE_GUARD_END()