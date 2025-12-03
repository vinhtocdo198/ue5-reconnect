// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/Nodes/SelectAutomatedTestingEOSAccountNode.h"

#include "RedpointEOSAuth/AuthenticationGraphState.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3077486437, Redpoint::EOS::Auth::Nodes)
{

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

#if !defined(UE_BUILD_SHIPPING) || !UE_BUILD_SHIPPING

void FSelectAutomatedTestingEOSAccountNode::Execute(
    TSharedRef<FAuthenticationGraphState> State,
    FAuthenticationGraphNodeOnDone OnDone)
{
    for (const auto &Candidate : State->EOSCandidates)
    {
        if (Candidate.DisplayName.ToString() == TEXT("AutomatedTesting"))
        {
            if (State->HasSelectedEOSCandidate())
            {
                State->ErrorMessages.Add(
                    TEXT("SelectAutomatedTestingEOSAccountNode hit with multiple valid candidates"));
                OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
                return;
            }
            State->SelectEOSCandidate(Candidate);
        }
    }

    if (!State->HasSelectedEOSCandidate())
    {
        State->ErrorMessages.Add(TEXT("SelectAutomatedTestingEOSAccountNode hit with no valid candidates"));
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
        return;
    }

    OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
}

#endif

#endif

}

REDPOINT_EOS_CODE_GUARD_END()