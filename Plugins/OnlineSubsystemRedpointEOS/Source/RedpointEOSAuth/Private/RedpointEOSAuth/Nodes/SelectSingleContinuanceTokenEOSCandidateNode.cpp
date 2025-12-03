// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/Nodes/SelectSingleContinuanceTokenEOSCandidateNode.h"

#include "RedpointEOSAuth/AuthenticationGraphState.h"
#include "RedpointEOSCore/Legacy/EOSString.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(4072958602, Redpoint::EOS::Auth::Nodes)
{

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

void FSelectSingleContinuanceTokenEOSCandidateNode::Execute(
    TSharedRef<FAuthenticationGraphState> State,
    FAuthenticationGraphNodeOnDone OnDone)
{
    for (const auto &Candidate : State->EOSCandidates)
    {
        if (!EOSString_ContinuanceToken::IsNone(Candidate.ContinuanceToken))
        {
            if (State->HasSelectedEOSCandidate())
            {
                State->ErrorMessages.Add(
                    TEXT("SelectSingleContinuanceTokenEOSCandidateNode hit with multiple valid candidates"));
                OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
                return;
            }
            State->SelectEOSCandidate(Candidate);
        }
    }

    if (!State->HasSelectedEOSCandidate())
    {
        State->ErrorMessages.Add(TEXT("SelectSingleContinuanceTokenEOSCandidateNode hit with no valid candidates"));
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
        return;
    }

    OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
}

#endif

}

REDPOINT_EOS_CODE_GUARD_END()