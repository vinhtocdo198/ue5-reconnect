// Copyright June Rhodes. All Rights Reserved.

#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepSearchingSendFollowRequest.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepSearchingSendFollowRequestMoveToWaitingStatus.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

TSharedPtr<FMatchmakingAlgorithmStepStateSearchingSendFollowRequest>
FMatchmakingAlgorithmStepSearchingSendFollowRequest::CreateNewState()
{
    return MakeShared<FMatchmakingAlgorithmStepStateSearchingSendFollowRequest>();
}

void FMatchmakingAlgorithmStepSearchingSendFollowRequest::Start(
    TSharedRef<FMatchmakingEngineTask> Task,
    FMatchmakingSuitableMatchResult MatchCandidate)
{
    EMIT_TASK_COUNTER_TRACE(Task);
    this->MoveToStep(*Task, MMSTEP_SEARCHING_SEND_FOLLOW_REQUEST_MOVE_TO_WAITING_STATUS, MatchCandidate);
}

#endif

REDPOINT_EOS_CODE_GUARD_END()