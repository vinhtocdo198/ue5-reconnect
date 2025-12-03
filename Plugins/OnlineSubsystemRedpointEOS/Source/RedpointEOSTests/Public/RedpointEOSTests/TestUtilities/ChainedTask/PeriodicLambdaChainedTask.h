// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/ChainedTask.h"

namespace REDPOINT_EOS_FILE_NS_ID(1225846553, Redpoint::EOS::Tests::TestUtilities::ChainedTask)
{
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

class REDPOINTEOSTESTS_API FPeriodicLambdaChainedTask : public FChainedTask,
                                                        public TSharedFromThis<FPeriodicLambdaChainedTask>
{
public:
    typedef TDelegate<void(const FChainedTaskContextRef &Context, int Iteration, const FOnComplete &OnDone)> FOnExecute;

private:
    float DelaySeconds;
    int CurrentIteration;
    int TotalIterations;
    FOnExecute OnExecute;

    FOnComplete OnDone;

    void ReceiveIterationDone(
        bool bContinueTesting,
        TSharedRef<FPeriodicLambdaChainedTask> This,
        FChainedTaskContextRef Context);
    bool ReceiveDelayComplete(
        float DeltaSeconds,
        TSharedRef<FPeriodicLambdaChainedTask> This,
        FChainedTaskContextRef Context);

public:
    template <typename FunctorType, typename... VarTypes>
    FPeriodicLambdaChainedTask(float InDelaySeconds, int InIterations, FunctorType &&InFunctor, VarTypes &&...Vars)
        : DelaySeconds(InDelaySeconds)
        , CurrentIteration(0)
        , TotalIterations(InIterations)
        , OnExecute(FPeriodicLambdaChainedTask::FOnExecute::CreateLambda(InFunctor, Vars...))
        , OnDone()
    {
    }

    UE_NONCOPYABLE(FPeriodicLambdaChainedTask);
    virtual ~FPeriodicLambdaChainedTask() = default;

    virtual void ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone);
};

}

namespace Redpoint::EOS::Tests::TestUtilities::ChainedTask
{
REDPOINT_EOS_FILE_NS_EXPORT(1225846553, Redpoint::EOS::Tests::TestUtilities::ChainedTask, FPeriodicLambdaChainedTask)
}