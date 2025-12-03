// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "../MatchmakingEngineImpl.h"
#include "../MatchmakingEngineTask.h"
#include "../MatchmakingProgressCalculator.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMSTEP_ERROR FName("Error")

class IMatchmakingAlgorithmStepState
{
};

template <typename TState, typename... TParams> class IMatchmakingAlgorithmStep
{
    friend class FMatchmakingEngineImpl;

    static_assert(
        std::is_base_of<IMatchmakingAlgorithmStepState, TState>::value,
        "TState must derive from IMatchmakingAlgorithmStepState");
    static_assert(std::is_class<TState>::value, "TState must be a class");

private:
    TWeakPtr<class FMatchmakingEngineImpl> Engine;

protected:
    /**
     * Returns our state from the task.
     */
    TSharedPtr<TState> GetState(FMatchmakingEngineTask &Task)
    {
        TSharedPtr<IMatchmakingAlgorithmStepState> State = Task.StepStates[GetName()];
        return StaticCastSharedPtr<TState>(State);
    }

    /**
     * Gets the log prefix to use for logging.
     */
    FString GetLogPrefix(const FMatchmakingEngineTask &Task) const
    {
        return FString::Printf(TEXT("Task %s: %s: "), *Task.TaskId, *this->GetName().ToString());
    }

    /**
     * The task has encountered a permanent error and can not continue.
     */
    void PermanentError(FMatchmakingEngineTask &Task, const FOnlineError &Error)
    {
        checkf(!Error.ErrorCode.IsEmpty(), TEXT("Passed error didn't have an ErrorCode in PermanentError"));
        UE_LOG(
            LogMatchmaking,
            Verbose,
            TEXT("%sError before moving to MMSTEP_ERROR: %s"),
            *this->GetLogPrefix(Task),
            *Error.ToLogString());
        this->MoveToStep<FOnlineError>(Task, MMSTEP_ERROR, Error);
    }

    /**
     * The task has encountered a critical error that can be recovered by restarting the matchmaking process.
     */
    void RestartableError(FMatchmakingEngineTask &Task, const FString &ErrorMessage)
    {
        FOnlineError Error = FOnlineError::CreateError(
            TEXT("errors.com.redpoint.matchmaking"),
            EOnlineErrorResult::RequestFailure,
            TEXT("errors.com.redpoint.matchmaking.restartable_error"),
            FText::FromString(ErrorMessage));
        UE_LOG(
            LogMatchmaking,
            Verbose,
            TEXT("%sError before moving to MMSTEP_ERROR: %s"),
            *this->GetLogPrefix(Task),
            *Error.ToLogString());
        checkf(!Error.ErrorCode.IsEmpty(), TEXT("Error construction incorrect in RestartableError"));
        this->MoveToStep<FOnlineError>(Task, MMSTEP_ERROR, Error);
    }

public:
    virtual ~IMatchmakingAlgorithmStep() = default;
    UE_NONCOPYABLE(IMatchmakingAlgorithmStep);
    IMatchmakingAlgorithmStep() = default;

    /**
     * Moves the task to another step.
     */
    template <typename... TOtherParams>
    void MoveToStep(FMatchmakingEngineTask &Task, FName StepName, TOtherParams... Parameters)
    {
        checkf(
            Task.CurrentStep.IsEqual(GetName()),
            TEXT("Matchmaking: Attempted to MoveToStep from step '%s' that is not current step '%s'"),
            *GetName().ToString(),
            *Task.CurrentStep.ToString());

        EMIT_TASK_COUNTER_TRACE(Task.AsShared());

        TSharedPtr<IMatchmakingAlgorithmStep<IMatchmakingAlgorithmStepState, TOtherParams...>> Step;
        if (TSharedPtr<FMatchmakingEngineImpl> PinnedEngine = this->Engine.Pin())
        {
            Step = PinnedEngine
                       ->GetRegisteredStep<IMatchmakingAlgorithmStep<IMatchmakingAlgorithmStepState, TOtherParams...>>(
                           StepName);
        }
        else
        {
            checkf(false, TEXT("Matchmaking: Engine instance has been free during MoveToStep"));
        }
        Task.CurrentStep = StepName;
        if (!Task.StepStates.Contains(StepName))
        {
            Task.StepStates.Add(StepName, StaticCastSharedPtr<IMatchmakingAlgorithmStepState>(Step->CreateNewState()));
        }

        if (Task.Request.OnProgress.IsBound())
        {
            FMatchmakingEngineProgressInfo ProgressInfo = {};
            ProgressInfo.Handle = Task.Handle;
            ProgressInfo.UserId = Task.Request.UserId;
            ProgressInfo.PartyId = Task.Request.PartyId;
            ProgressInfo.StepName = Step->GetName();
            ProgressInfo.CurrentStatus = Step->GetStatus();
            ProgressInfo.CurrentDetail = Step->GetDetail();
            ProgressInfo.CurrentProgress = FMatchmakingProgressCalculator::ComputeProgress(Task);
            ProgressInfo.EstimatedTimeOfCompletion = Task.EstimatedTimeOfCompletion;
            Task.Request.OnProgress.ExecuteIfBound(Task.TaskId, Task.Request, ProgressInfo);
            EMIT_TASK_COUNTER_TRACE(Task.AsShared());

            if (!Task.CurrentStep.IsEqual(StepName))
            {
                // If a user calls Cancel() inside the OnProgress callback, we might have jumped
                // immediately to a new Abandon step, in which case we don't want to call Start.
                return;
            }
        }

        EMIT_TASK_COUNTER_TRACE(Task.AsShared());
        Step->Start(Task.AsShared(), Parameters...);
        EMIT_TASK_COUNTER_TRACE(Task.AsShared());
    }

    /**
     * Gets the unique name of this step.
     */
    virtual FName GetName() const = 0;

    /**
     * Gets the status to display to the user when this step is the current step. This value should not change.
     */
    virtual FText GetStatus() const = 0;

    /**
     * Gets the detail to display to the user when this step is the current step. This value should not change.
     */
    virtual FText GetDetail() const = 0;

    /**
     * For tasks that don't have a state for this step, this function returns a new empty state object.
     */
    virtual TSharedPtr<TState> CreateNewState() = 0;

    /**
     * Requests the start of this step against the task.
     */
    virtual void Start(TSharedRef<FMatchmakingEngineTask> Task, TParams... Parameters) = 0;

    /**
     * Returns whether the specified interrupt can be handled in this frame. The result of this function
     * and the next call to HandleInterrupt in the same frame must match.
     *
     * Currently this is only used by the engine when it is returning whether or not matchmaking can
     * be cancelled. Event-based interrupts do not use this function.
     */
    virtual bool CanHandleInterrupt(const TSharedRef<FMatchmakingEngineTask> &Task, FName InterruptType)
    {
        return false;
    }

    /**
     * Handle the specified interrupt. If you return true, the caller will change the current step, so
     * you must cleanup any in-progress operations.
     */
    virtual bool HandleInterrupt(const TSharedRef<FMatchmakingEngineTask> &Task, FName InterruptType)
    {
        return false;
    }
};

#endif

REDPOINT_EOS_CODE_GUARD_END()