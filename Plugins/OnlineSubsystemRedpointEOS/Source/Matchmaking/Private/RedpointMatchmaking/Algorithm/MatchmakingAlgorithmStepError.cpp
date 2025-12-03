// Copyright June Rhodes. All Rights Reserved.

#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepError.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepAbandonStart.h"
#include "RedpointMatchmaking/MatchmakingLog.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

TSharedPtr<FMatchmakingAlgorithmStepStateError> FMatchmakingAlgorithmStepError::CreateNewState()
{
    return MakeShared<FMatchmakingAlgorithmStepStateError>();
}

void FMatchmakingAlgorithmStepError::Start(TSharedRef<FMatchmakingEngineTask> Task, FOnlineError Error)
{
    if (Error.ErrorCode.EndsWith(TEXT(".restartable_error")))
    {
        UE_LOG(
            LogMatchmaking,
            Error,
            TEXT("Encountered restartable error while matchmaking: %s"),
            *Error.ErrorMessage.ToString());
        this->MoveToStep(*Task, MMSTEP_ABANDON_START, EMatchmakingAbandonmentType::Restart);
    }
    else
    {
        UE_LOG(LogMatchmaking, Error, TEXT("Encountered permanent error while matchmaking: %s"), *Error.ToLogString());
        Task->Request.OnError.ExecuteIfBound(Task->TaskId, Task->Request, Error);
    }
}

#endif

REDPOINT_EOS_CODE_GUARD_END()