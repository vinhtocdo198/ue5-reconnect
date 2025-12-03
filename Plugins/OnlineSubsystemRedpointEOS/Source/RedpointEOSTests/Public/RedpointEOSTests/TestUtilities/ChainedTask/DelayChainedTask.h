// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/ChainedTask.h"

namespace REDPOINT_EOS_FILE_NS_ID(1629354960, Redpoint::EOS::Tests::TestUtilities::ChainedTask)
{
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

class REDPOINTEOSTESTS_API FDelayChainedTask : public FChainedTask, public TSharedFromThis<FDelayChainedTask>
{
private:
    float DelaySeconds;
    bool ReceiveDelayComplete(float DeltaSeconds, TSharedRef<FDelayChainedTask> This, FOnComplete OnDone);

public:
    FDelayChainedTask(float InDelaySeconds);
    UE_NONCOPYABLE(FDelayChainedTask);
    virtual ~FDelayChainedTask() = default;

    virtual void ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone);
};

}

namespace Redpoint::EOS::Tests::TestUtilities::ChainedTask
{
REDPOINT_EOS_FILE_NS_EXPORT(1629354960, Redpoint::EOS::Tests::TestUtilities::ChainedTask, FDelayChainedTask)
}