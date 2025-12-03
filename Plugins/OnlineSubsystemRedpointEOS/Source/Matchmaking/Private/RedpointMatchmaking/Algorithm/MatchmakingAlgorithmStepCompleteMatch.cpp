// Copyright June Rhodes. All Rights Reserved.

#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepCompleteMatch.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepCompleteMatchComputeResult.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepDeleteMatchmakingFollowRequestLobby.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

TSharedPtr<FMatchmakingAlgorithmStepStateCompleteMatch> FMatchmakingAlgorithmStepCompleteMatch::CreateNewState()
{
    return MakeShared<FMatchmakingAlgorithmStepStateCompleteMatch>();
}

void FMatchmakingAlgorithmStepCompleteMatch::Start(TSharedRef<FMatchmakingEngineTask> Task)
{
    if (Task->MatchmakingFollowRequestLobbyId.IsValid())
    {
        // We need to cleanup the matchmaking follow request lobby first.
        EMIT_TASK_COUNTER_TRACE(Task);
        this->MoveToStep(*Task, MMSTEP_DELETE_MATCHMAKING_FOLLOW_REQUEST_LOBBY, MMSTEP_COMPLETE_MATCH_COMPUTE_RESULT);
    }
    else
    {
        // Immediately start computing the result.
        EMIT_TASK_COUNTER_TRACE(Task);
        this->MoveToStep(*Task, MMSTEP_COMPLETE_MATCH_COMPUTE_RESULT);
    }
}

#endif

REDPOINT_EOS_CODE_GUARD_END()