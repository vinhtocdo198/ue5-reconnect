// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Async/TaskGraphInterfaces.h"
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(881433052, Redpoint::EOS::Core::Utils)
{
using namespace ::Redpoint::EOS::API;

class REDPOINTEOSCORE_API FAsyncTask : public TSharedFromThis<FAsyncTask>
{
public:
    FGraphEventRef AsyncComplete;
    FGraphEventRef Job;
    TSharedRef<FError> Error;
    bool bCompleted;

    FAsyncTask();
    UE_NONCOPYABLE(FAsyncTask);
    ~FAsyncTask();
};

typedef TSharedRef<FAsyncTask> FAsyncTaskRef;
typedef TSharedPtr<FAsyncTask> FAsyncTaskPtr;

class REDPOINTEOSCORE_API FAsyncTaskGraph : public TSharedFromThis<FAsyncTaskGraph>
{
private:
    TStatId StatId;
    TArray<FAsyncTaskRef> Tasks;
    FGraphEventRef StartRef;
    FGraphEventRef FinallyRef;
    FGraphEventRef CompleteRef;
    TArray<FAsyncTaskRef> FinallyTasks;
    bool bFinallyDispatched;
    TSharedPtr<FAsyncTaskGraph> SelfPtr;
    bool bStartCalled;

    FString GetGraphProgress() const;
    void MarkComplete(const FAsyncTaskRef &Task, const TSharedRef<FError> &Error);

public:
    FAsyncTaskGraph(const TStatId &InStatId);
    FAsyncTaskGraph();
    UE_NONCOPYABLE(FAsyncTaskGraph);
    ~FAsyncTaskGraph();

    typedef TDelegate<void(const FError &Error)> FOnComplete;
    typedef TDelegate<void(const FOnComplete &OnComplete)> FOnExecuteTask;
    typedef TDelegate<void(const FError &Error)> FOnExecuteFinally;

    enum ENoDependencies
    {
        StartImmediately
    };

    FAsyncTaskRef AddDelegate(const FOnExecuteTask &OnExecuteTask, ENoDependencies InNoDependencies);
    FAsyncTaskRef AddDelegate(const FOnExecuteTask &OnExecuteTask, const FAsyncTaskPtr &InDependentOn);
    FAsyncTaskRef AddDelegate(
        const FOnExecuteTask &OnExecuteTask,
        const std::initializer_list<FAsyncTaskRef> &InDependentOn);
    FAsyncTaskRef AddDelegate(const FOnExecuteTask &OnExecuteTask, const TArray<FAsyncTaskRef> &InDependentOn);
    FAsyncTaskRef FinallyDelegate(const FOnExecuteFinally &OnExecuteFinally);

    void Start();

    template <typename CallableType> FAsyncTaskRef Add(CallableType &&Callable, ENoDependencies InNoDependencies)
    {
        return this->AddDelegate(FOnExecuteTask::CreateLambda(Callable), InNoDependencies);
    }
    template <typename CallableType> FAsyncTaskRef Add(CallableType &&Callable, const FAsyncTaskPtr &InDependentOn)
    {
        return this->AddDelegate(FOnExecuteTask::CreateLambda(Callable), InDependentOn);
    }
    template <typename CallableType>
    FAsyncTaskRef Add(CallableType &&Callable, const std::initializer_list<FAsyncTaskRef> &InDependentOn)
    {
        return this->AddDelegate(FOnExecuteTask::CreateLambda(Callable), InDependentOn);
    }
    template <typename CallableType>
    FAsyncTaskRef Add(CallableType &&Callable, const TArray<FAsyncTaskRef> &InDependentOn)
    {
        return this->AddDelegate(FOnExecuteTask::CreateLambda(Callable), InDependentOn);
    }
    template <typename CallableType> FAsyncTaskRef Finally(CallableType &&Callable)
    {
        return this->FinallyDelegate(FOnExecuteFinally::CreateLambda(Callable));
    }
};

}

namespace Redpoint::EOS::Core::Utils
{
REDPOINT_EOS_FILE_NS_EXPORT(881433052, Redpoint::EOS::Core::Utils, FAsyncTask)
REDPOINT_EOS_FILE_NS_EXPORT(881433052, Redpoint::EOS::Core::Utils, FAsyncTaskRef)
REDPOINT_EOS_FILE_NS_EXPORT(881433052, Redpoint::EOS::Core::Utils, FAsyncTaskPtr)
REDPOINT_EOS_FILE_NS_EXPORT(881433052, Redpoint::EOS::Core::Utils, FAsyncTaskGraph)
}

REDPOINT_EOS_CODE_GUARD_END()