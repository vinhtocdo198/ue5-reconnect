// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSTests/TestUtilities/ChainedTask/LambdaChainedTask.h"

namespace REDPOINT_EOS_FILE_NS_ID(2672503325, Redpoint::EOS::Tests::TestUtilities::ChainedTask)
{
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

FLambdaChainedTask::FLambdaChainedTask(const FOnExecute &InOnExecute)
    : OnExecute(InOnExecute)
{
}

void FLambdaChainedTask::ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone)
{
    if (this->OnExecute.IsBound())
    {
        this->OnExecute.Execute(Context, OnDone);
    }
    else
    {
        Context->Assert().TestTrue("OnExecute was not bound for LambdaChainedTask!", false);
        OnDone.ExecuteIfBound(false);
    }
}

}