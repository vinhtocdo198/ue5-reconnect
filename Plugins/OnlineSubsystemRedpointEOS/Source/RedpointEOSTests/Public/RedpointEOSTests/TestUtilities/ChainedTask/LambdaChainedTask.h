// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/ChainedTask.h"

namespace REDPOINT_EOS_FILE_NS_ID(2672503325, Redpoint::EOS::Tests::TestUtilities::ChainedTask)
{
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

class REDPOINTEOSTESTS_API FLambdaChainedTask : public FChainedTask
{
public:
    typedef TDelegate<void(const FChainedTaskContextRef &Context, const FOnComplete &OnDone)> FOnExecute;

private:
    FOnExecute OnExecute;

public:
    FLambdaChainedTask(const FOnExecute &InOnExecute);

    template <typename FunctorType, typename... VarTypes>
    FLambdaChainedTask(FunctorType &&InFunctor, VarTypes &&...Vars)
        : OnExecute(FLambdaChainedTask::FOnExecute::CreateLambda(InFunctor, Vars...))
    {
    }

    UE_NONCOPYABLE(FLambdaChainedTask);
    virtual ~FLambdaChainedTask() = default;

    virtual void ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone);
};

}

namespace Redpoint::EOS::Tests::TestUtilities::ChainedTask
{
REDPOINT_EOS_FILE_NS_EXPORT(2672503325, Redpoint::EOS::Tests::TestUtilities::ChainedTask, FLambdaChainedTask)
}