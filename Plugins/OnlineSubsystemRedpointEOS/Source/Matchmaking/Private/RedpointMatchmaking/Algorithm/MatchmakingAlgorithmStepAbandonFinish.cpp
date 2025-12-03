// Copyright June Rhodes. All Rights Reserved.

#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepAbandonFinish.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepValidateRequest.h"
#include "RedpointMatchmaking/EventHandlers/MatchmakingEventHandlerAbandonment.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

TSharedPtr<FMatchmakingAlgorithmStepStateAbandonFinish> FMatchmakingAlgorithmStepAbandonFinish::CreateNewState()
{
    return MakeShared<FMatchmakingAlgorithmStepStateAbandonFinish>();
}

void FMatchmakingAlgorithmStepAbandonFinish::Start(
    TSharedRef<FMatchmakingEngineTask> Task,
    EMatchmakingAbandonmentType AbandonmentType)
{
    // Re-create the task manager. We can't just reset it's state, because a host following another host
    // causes it's team manager to be released. Therefore, we need to make sure we create it like we do
    // when a task is first created.
    Task->TeamManager = Task->Request.HostConfiguration.IsValid()
                            ? TSharedPtr<FMatchmakingEngineTaskTeamManager>(
                                  MakeShared<FMatchmakingEngineTaskTeamManager>(Task->Request))
                            : nullptr;

    if (AbandonmentType == EMatchmakingAbandonmentType::Restart)
    {
        UE_LOG(LogMatchmaking, Verbose, TEXT("Abandonment finished, now restarting matchmaking..."));

        // Just start at the beginning again.
        this->MoveToStep(*Task, MMSTEP_VALIDATE_REQUEST);
    }
    else
    {
        UE_LOG(LogMatchmaking, Verbose, TEXT("Abandonment finished, now cancelling matchmaking..."));

        Task->Request.OnCancelled.ExecuteIfBound(Task->TaskId, Task->Request);
    }
}

bool FMatchmakingAlgorithmStepAbandonFinish::CanHandleInterrupt(
    const TSharedRef<FMatchmakingEngineTask> &Task,
    FName InterruptType)
{
    if (InterruptType.IsEqual(MMINTERRUPT_HAS_SPECIAL_HANDLING_OF_ABANDONMENT) ||
        InterruptType.IsEqual(MMINTERRUPT_ABANDON_CANCEL) || InterruptType.IsEqual(MMINTERRUPT_ABANDON_RESTART))
    {
        return true;
    }

    return false;
}

bool FMatchmakingAlgorithmStepAbandonFinish::HandleInterrupt(
    const TSharedRef<FMatchmakingEngineTask> &Task,
    FName InterruptType)
{
    if (InterruptType.IsEqual(MMINTERRUPT_HAS_SPECIAL_HANDLING_OF_ABANDONMENT) ||
        InterruptType.IsEqual(MMINTERRUPT_ABANDON_CANCEL) || InterruptType.IsEqual(MMINTERRUPT_ABANDON_RESTART))
    {
        // We're already abandoning.
        return true;
    }

    return false;
}

#endif

REDPOINT_EOS_CODE_GUARD_END()