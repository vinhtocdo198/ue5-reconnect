// Copyright June Rhodes. All Rights Reserved.

#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepCompleteMatchFireEvent.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

TSharedPtr<FMatchmakingAlgorithmStepStateCompleteMatchFireEvent> FMatchmakingAlgorithmStepCompleteMatchFireEvent::
    CreateNewState()
{
    return MakeShared<FMatchmakingAlgorithmStepStateCompleteMatchFireEvent>();
}

void FMatchmakingAlgorithmStepCompleteMatchFireEvent::Start(TSharedRef<FMatchmakingEngineTask> Task)
{
    checkf(
        Task->StoredResponse.IsValid(),
        TEXT("Expected StoredResponse to be valid when CompleteMatchFireEvent runs!"));

    Task->Request.CompletionBehaviour.OnComplete.ExecuteIfBound(Task->TaskId, Task->Request, *Task->StoredResponse);
}

#endif

REDPOINT_EOS_CODE_GUARD_END()