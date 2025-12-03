// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/EventHandlers/MatchmakingEventHandler.h"
#include "RedpointMatchmaking/MatchmakingEngine.h"
#include "RedpointMatchmaking/MatchmakingEngineTask.h"
#include "Tickable.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

class FMatchmakingEngineImpl : public IMatchmakingEngine,
                               public TSharedFromThis<FMatchmakingEngineImpl>,
                               public FTickableGameObject
{
private:
    FMatchmakingEngineRequestHandle NextHandle;
    TMap<FName, TSharedPtr<void>> RegisteredSteps;
    TMap<FMatchmakingEngineRequestHandle, TSharedPtr<FMatchmakingEngineTask>> QueuedTasks;
    TArray<TSharedPtr<IMatchmakingEventHandler>> RegisteredEventHandlers;

    struct FScheduledCallback
    {
        float TargetElapsedTime;
        FDelegateHandle DelegateHandle;
        FMatchmakingEngineTaskScheduleCallback Callback;
    };

    bool bIsInstantScheduling;
    float ElapsedTime;
    TArray<FScheduledCallback> ScheduledCallbacks;

    template <typename T> void RegisterStep()
    {
        TSharedPtr<T> Step = MakeShared<T>();
        Step->Engine = this->AsShared();
        this->RegisteredSteps.Add(Step->GetName(), Step);
    }

public:
    FMatchmakingEngineImpl(bool bInIsInstantScheduling);
    UE_NONCOPYABLE(FMatchmakingEngineImpl);
    virtual ~FMatchmakingEngineImpl() override = default;

private:
    bool OnHostSendHeartbeat(float DeltaSeconds, TWeakPtr<class FMatchmakingEngineTask> TaskWk);

    void PrepareForTaskComplete(
        const FString &TaskId,
        const struct FMatchmakingEngineRequest &Request,
        FMatchmakingEngineResponse Response,
        FMatchmakingEngineRequestHandle Handle,
        FMatchmakingEngineRequestComplete OriginalOnComplete);
    void ExecuteTaskComplete(
        FString TaskId,
        FMatchmakingEngineRequest Request,
        FMatchmakingEngineResponse Response,
        FMatchmakingEngineRequestHandle Handle,
        FMatchmakingEngineRequestComplete OriginalOnComplete);

    void PrepareForTaskCancelled(
        const FString &TaskId,
        const struct FMatchmakingEngineRequest &Request,
        FMatchmakingEngineRequestHandle Handle,
        FMatchmakingEngineRequestCancelled OriginalOnCancelled);
    void ExecuteTaskCancelled(
        FString TaskId,
        FMatchmakingEngineRequest Request,
        FMatchmakingEngineRequestHandle Handle,
        FMatchmakingEngineRequestCancelled OriginalOnCancelled);

    void PrepareForTaskError(
        const FString &TaskId,
        const struct FMatchmakingEngineRequest &Request,
        const FOnlineError &Error,
        FMatchmakingEngineRequestHandle Handle,
        FMatchmakingEngineRequestError OriginalOnError);
    void ExecuteTaskError(
        FString TaskId,
        FMatchmakingEngineRequest Request,
        FOnlineError Error,
        FMatchmakingEngineRequestHandle Handle,
        FMatchmakingEngineRequestError OriginalOnError);

public:
    virtual bool IsTickableWhenPaused() const override
    {
        return true;
    }
    virtual bool IsTickableInEditor() const override
    {
        return true;
    }

    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override
    {
        RETURN_QUICK_DECLARE_CYCLE_STAT(FMatchmakingEngineImpl, STATGROUP_Tickables);
    }

    void RegisterSteps();

    template <typename T> TSharedRef<T> GetRegisteredStep(FName StepName)
    {
        return StaticCastSharedPtr<T>(this->RegisteredSteps[StepName]).ToSharedRef();
    }

    FDelegateHandle OnSchedule(float Delay, FMatchmakingEngineTaskScheduleCallback Callback);
    void OnUnschedule(FDelegateHandle Handle);
    bool IsInstantScheduling();

    virtual FMatchmakingEngineRequestHandle Enqueue(const FMatchmakingEngineRequest Request) override;
    virtual void Cancel(FMatchmakingEngineRequestHandle Handle) override;
};

#endif

REDPOINT_EOS_CODE_GUARD_END()