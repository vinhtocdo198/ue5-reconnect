// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSTests/TestUtilities/ChainedTask/PeriodicLambdaChainedTask.h"

#include "Containers/Ticker.h"

namespace REDPOINT_EOS_FILE_NS_ID(1225846553, Redpoint::EOS::Tests::TestUtilities::ChainedTask)
{
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

void FPeriodicLambdaChainedTask::ReceiveIterationDone(
    bool bContinueTesting,
    TSharedRef<FPeriodicLambdaChainedTask> This,
    FChainedTaskContextRef Context)
{
    this->CurrentIteration++;

    if (!bContinueTesting)
    {
        this->OnDone.ExecuteIfBound(false);
        return;
    }

    if (this->CurrentIteration >= this->TotalIterations)
    {
        // We're actually done.
        this->OnDone.ExecuteIfBound(true);
    }
    else
    {
        FTSTicker::GetCoreTicker().AddTicker(
            FTickerDelegate::CreateSP(this, &FPeriodicLambdaChainedTask::ReceiveDelayComplete, This, Context),
            this->DelaySeconds);
    }
}

bool FPeriodicLambdaChainedTask::ReceiveDelayComplete(
    float DeltaSeconds,
    TSharedRef<FPeriodicLambdaChainedTask> This,
    FChainedTaskContextRef Context)
{
    // Start the next loop iteration.
    this->OnExecute.Execute(
        Context,
        this->CurrentIteration,
        FOnComplete::CreateSP(this, &FPeriodicLambdaChainedTask::ReceiveIterationDone, This, Context));

    // This always only fires once.
    return false;
}

void FPeriodicLambdaChainedTask::ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &InOnDone)
{
    if (!this->OnExecute.IsBound())
    {
        Context->Assert().TestTrue("OnExecute was not bound for PeriodicLambdaChainedTask!", false);
        InOnDone.ExecuteIfBound(false);
        return;
    }

    this->OnDone = InOnDone;
    this->CurrentIteration = 0;
    this->OnExecute.Execute(
        Context,
        this->CurrentIteration,
        FOnComplete::CreateSP(this, &FPeriodicLambdaChainedTask::ReceiveIterationDone, this->AsShared(), Context));
}

}