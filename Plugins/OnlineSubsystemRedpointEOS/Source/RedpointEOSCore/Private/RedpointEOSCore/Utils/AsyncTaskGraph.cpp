// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSCore/Utils/AsyncTaskGraph.h"

#include "Containers/Ticker.h"
#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSCore/Logging.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(881433052, Redpoint::EOS::Core::Utils)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Core;

FAsyncTask::FAsyncTask()
    : AsyncComplete(FGraphEvent::CreateGraphEvent())
    , Job(nullptr)
    , Error(MakeShared<FError>(Errors::Success()))
    , bCompleted(false)
{
}

FAsyncTask::~FAsyncTask()
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    checkf(this->Job->IsComplete(), TEXT("Job should have been completed before FAsyncTask is destroyed."));
    checkf(
        this->AsyncComplete->IsComplete(),
        TEXT("AsyncComplete should have been completed before FAsyncTask is destroyed."));
}

FString FAsyncTaskGraph::GetGraphProgress() const
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    int TaskComplete = 0;
    int FinallyComplete = 0;
    for (const auto &Task : this->Tasks)
    {
        if (Task->AsyncComplete->IsComplete())
        {
            TaskComplete++;
        }
    }
    for (const auto &Task : this->FinallyTasks)
    {
        if (Task->AsyncComplete->IsComplete())
        {
            FinallyComplete++;
        }
    }
    return FString::Printf(
        TEXT("(%d/%d tasks) (%d/%d finally)"),
        TaskComplete,
        this->Tasks.Num(),
        FinallyComplete,
        this->FinallyTasks.Num());
}

void FAsyncTaskGraph::MarkComplete(const FAsyncTaskRef &Task, const TSharedRef<FError> &Error)
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    // Mark this task as complete.
    UE_LOG(
        LogRedpointEOSCore,
        VeryVerbose,
        TEXT("%p: FAsyncTaskGraph::MarkComplete: %s: %p: Marking as completed."),
        this,
        *this->GetGraphProgress(),
        &Task.Get());
    Task->Error = Error;
    Task->bCompleted = true;
    Task->AsyncComplete->DispatchSubsequents();

    // Are all tasks complete?
    if (!this->bFinallyDispatched)
    {
        for (const auto &TrackedTask : this->Tasks)
        {
            if (!TrackedTask->bCompleted)
            {
                // We shouldn't fire 'finally' yet.
                return;
            }
        }
        UE_LOG(
            LogRedpointEOSCore,
            VeryVerbose,
            TEXT("%p: FAsyncTaskGraph::MarkComplete: %s: Dispatching finally."),
            this,
            *this->GetGraphProgress());
        this->bFinallyDispatched = true;
        this->FinallyRef->DispatchSubsequents();
    }
}

FAsyncTaskGraph::FAsyncTaskGraph(const TStatId &InStatId)
    : StatId(InStatId)
    , Tasks()
    , StartRef(FGraphEvent::CreateGraphEvent())
    , FinallyRef(FGraphEvent::CreateGraphEvent())
    , FinallyTasks()
    , bFinallyDispatched(false)
    , SelfPtr()
    , bStartCalled(false)
{
}

FAsyncTaskGraph::FAsyncTaskGraph()
    : StatId()
    , Tasks()
    , StartRef(FGraphEvent::CreateGraphEvent())
    , FinallyRef(FGraphEvent::CreateGraphEvent())
    , FinallyTasks()
    , bFinallyDispatched(false)
    , SelfPtr()
    , bStartCalled(false)
{
}

FAsyncTaskGraph::~FAsyncTaskGraph()
{
#if DO_CHECK
    REDPOINT_EOS_USE_LLM_TAG(Core);

    checkf(this->bStartCalled, TEXT("Expected Start to be called."));
    for (const auto &Task : this->Tasks)
    {
        checkf(Task->AsyncComplete->IsComplete(), TEXT("Expected task to be completed before graph destroyed!"));
    }
    for (const auto &Task : this->FinallyTasks)
    {
        checkf(Task->AsyncComplete->IsComplete(), TEXT("Expected task to be completed before graph destroyed!"));
    }
    checkf(this->StartRef->IsComplete(), TEXT("Expected StartRef to be completed."));
    checkf(this->FinallyRef->IsComplete(), TEXT("Expected FinallyRef to be completed."));
#endif
}

FAsyncTaskRef FAsyncTaskGraph::AddDelegate(const FOnExecuteTask &OnExecuteTask, ENoDependencies InNoDependencies)
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    return this->AddDelegate(OnExecuteTask, TArray<FAsyncTaskRef>());
}

FAsyncTaskRef FAsyncTaskGraph::AddDelegate(const FOnExecuteTask &OnExecuteTask, const FAsyncTaskPtr &InDependentOn)
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    TArray<FAsyncTaskRef> Dependents;
    if (InDependentOn.IsValid())
    {
        Dependents.Add(InDependentOn.ToSharedRef());
    }
    return this->AddDelegate(OnExecuteTask, Dependents);
}

FAsyncTaskRef FAsyncTaskGraph::AddDelegate(
    const FOnExecuteTask &OnExecuteTask,
    const std::initializer_list<FAsyncTaskRef> &InDependentOn)
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    TArray<FAsyncTaskRef> Dependents;
    for (const auto &Dependent : InDependentOn)
    {
        Dependents.Add(Dependent);
    }
    return this->AddDelegate(OnExecuteTask, Dependents);
}

FAsyncTaskRef FAsyncTaskGraph::AddDelegate(
    const FOnExecuteTask &OnExecuteTask,
    const TArray<FAsyncTaskRef> &InDependentOn)
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    FGraphEventArray Prerequisites;
    Prerequisites.Add(this->StartRef);
    for (const auto &DependentOn : InDependentOn)
    {
        Prerequisites.Add(DependentOn->AsyncComplete);
        checkf(this->Tasks.Contains(DependentOn), TEXT("Expected dependency task to have previously been added."));
    }

    FAsyncTaskRef Task = MakeShared<FAsyncTask>();
    Task->Job = FFunctionGraphTask::CreateAndDispatchWhenReady(
        [This = this->AsShared(), Task, OnExecuteTask, InDependentOn] {
            UE_LOG(
                LogRedpointEOSCore,
                VeryVerbose,
                TEXT("%p: FAsyncTaskGraph::AddDelegate: %s: %p: Starting task."),
                &This.Get(),
                *This->GetGraphProgress(),
                &Task.Get());

            for (const auto &DependentOn : InDependentOn)
            {
                if (!(DependentOn->Error->WasSuccessful()))
                {
                    // Our dependent failed.
                    UE_LOG(
                        LogRedpointEOSCore,
                        VeryVerbose,
                        TEXT("%p: FAsyncTaskGraph::AddDelegate: %s: %p: Marking task as done (dependent failed)."),
                        &This.Get(),
                        *This->GetGraphProgress(),
                        &Task.Get());
                    This->MarkComplete(Task, DependentOn->Error);
                    return;
                }
            }

            if (!OnExecuteTask.IsBound())
            {
                // We don't have anything to call, so we fail.
                This->MarkComplete(
                    Task,
                    MakeShared<FError>(Errors::UnexpectedError(
                        ANSI_TO_TCHAR(__FUNCTION__),
                        TEXT("OnExecuteTask is no longer bound."))));
                return;
            }

            OnExecuteTask.Execute(FOnComplete::CreateLambda([This, Task](const FError &Error) {
                checkf(!Task->bCompleted, TEXT("OnComplete already called!"));

                UE_LOG(
                    LogRedpointEOSCore,
                    VeryVerbose,
                    TEXT("%p: FAsyncTaskGraph::AddDelegate: %s: %p: Marking task as done."),
                    &This.Get(),
                    *This->GetGraphProgress(),
                    &Task.Get());
                This->MarkComplete(Task, MakeShared<FError>(Error));
            }));
        },
        this->StatId,
        &Prerequisites,
        ENamedThreads::GameThread);
    this->Tasks.Add(Task);
    if (!this->SelfPtr.IsValid())
    {
        this->SelfPtr = this->AsShared();
    }
    UE_LOG(
        LogRedpointEOSCore,
        VeryVerbose,
        TEXT("%p: FAsyncTaskGraph::AddDelegate: %s: %p: Added task."),
        this,
        *this->GetGraphProgress(),
        &Task.Get());
    return Task;
}

FAsyncTaskRef FAsyncTaskGraph::FinallyDelegate(const FOnExecuteFinally &OnExecuteFinally)
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    FGraphEventArray Prerequisites;
    Prerequisites.Add(this->StartRef);
    Prerequisites.Add(this->FinallyRef);

    FAsyncTaskRef Task = MakeShared<FAsyncTask>();
    Task->Job = FFunctionGraphTask::CreateAndDispatchWhenReady(
        [This = this->AsShared(), Task, OnExecuteFinally] {
            UE_LOG(
                LogRedpointEOSCore,
                VeryVerbose,
                TEXT("%p: FAsyncTaskGraph::FinallyDelegate: %s: %p: Starting finally task."),
                &This.Get(),
                *This->GetGraphProgress(),
                &Task.Get());
            TSharedRef<FError> Error = MakeShared<FError>(Errors::Success());
            for (const auto &TrackedTask : This->Tasks)
            {
                if (!TrackedTask->Error->WasSuccessful())
                {
                    Error = TrackedTask->Error;
                    break;
                }
            }
            OnExecuteFinally.ExecuteIfBound(*Error);

            UE_LOG(
                LogRedpointEOSCore,
                VeryVerbose,
                TEXT("%p: FAsyncTaskGraph::FinallyDelegate: %s: %p: Marking finally task as done."),
                &This.Get(),
                *This->GetGraphProgress(),
                &Task.Get());
            Task->Error = MakeShared<FError>(Errors::Success());
            Task->bCompleted = true;
            Task->AsyncComplete->DispatchSubsequents();
        },
        this->StatId,
        &Prerequisites,
        ENamedThreads::GameThread);
    this->FinallyTasks.Add(Task);
    if (!this->SelfPtr.IsValid())
    {
        this->SelfPtr = this->AsShared();
    }
    UE_LOG(
        LogRedpointEOSCore,
        VeryVerbose,
        TEXT("%p: FAsyncTaskGraph::FinallyDelegate: %s: %p: Added finally."),
        this,
        *this->GetGraphProgress(),
        &Task.Get());
    return Task;
}

void FAsyncTaskGraph::Start()
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    checkf(!this->bStartCalled, TEXT("Start has already been called."));
    this->bStartCalled = true;

    FGraphEventArray Prerequisites;
    Prerequisites.Add(this->StartRef);
    Prerequisites.Add(this->FinallyRef);
    if (this->Tasks.Num() == 0)
    {
        // @note: Because of the way the task system works, we must always have one task to execute so that the
        // finally/complete tasks run as expected.
        this->Add(
            [](const FOnComplete &OnComplete) {
                OnComplete.ExecuteIfBound(Errors::Success());
            },
            FAsyncTaskGraph::ENoDependencies::StartImmediately);
    }
    for (const auto &Task : this->Tasks)
    {
        Prerequisites.Add(Task->AsyncComplete);
        Prerequisites.Add(Task->Job);
    }
    for (const auto &Task : this->FinallyTasks)
    {
        Prerequisites.Add(Task->AsyncComplete);
        Prerequisites.Add(Task->Job);
    }

    this->CompleteRef = FFunctionGraphTask::CreateAndDispatchWhenReady(
        [This = this->AsShared()] {
            // @note: We do this on delay so that this task (CompleteRef) will also have completed before the graph is
            // destructor is run.
            UE_LOG(
                LogRedpointEOSCore,
                VeryVerbose,
                TEXT("%p: FAsyncTaskGraph::Start: %s: Complete, destroying graph."),
                &This.Get(),
                *This->GetGraphProgress());
            FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([This](float DeltaSeconds) {
                This->SelfPtr.Reset();
                return false;
            }));
        },
        this->StatId,
        &Prerequisites,
        ENamedThreads::GameThread);
    if (!this->SelfPtr.IsValid())
    {
        this->SelfPtr = this->AsShared();
    }
    UE_LOG(
        LogRedpointEOSCore,
        VeryVerbose,
        TEXT("%p: FAsyncTaskGraph::Start: %s: Starting graph."),
        this,
        *this->GetGraphProgress());
    this->StartRef->DispatchSubsequents();
}

}

REDPOINT_EOS_CODE_GUARD_END()