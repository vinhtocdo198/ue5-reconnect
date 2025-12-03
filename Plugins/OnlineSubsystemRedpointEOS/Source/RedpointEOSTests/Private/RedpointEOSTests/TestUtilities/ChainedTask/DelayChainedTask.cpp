// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSTests/TestUtilities/ChainedTask/DelayChainedTask.h"

#include "Containers/Ticker.h"

namespace REDPOINT_EOS_FILE_NS_ID(1629354960, Redpoint::EOS::Tests::TestUtilities::ChainedTask)
{
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

bool FDelayChainedTask::ReceiveDelayComplete(
    float DeltaSeconds,
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    TSharedRef<FDelayChainedTask> This,
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    FOnComplete OnDone)
{
    OnDone.ExecuteIfBound(true);
    return false;
}

FDelayChainedTask::FDelayChainedTask(float InDelaySeconds)
    : DelaySeconds(InDelaySeconds)
{
}

void FDelayChainedTask::ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &InOnDone)
{
    FTSTicker::GetCoreTicker().AddTicker(
        FTickerDelegate::CreateSP(this, &FDelayChainedTask::ReceiveDelayComplete, this->AsShared(), InOnDone),
        this->DelaySeconds);
}

}