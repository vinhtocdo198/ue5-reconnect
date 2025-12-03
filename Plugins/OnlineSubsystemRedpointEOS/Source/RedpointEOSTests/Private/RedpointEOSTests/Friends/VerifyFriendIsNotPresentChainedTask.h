// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/ChainedTask.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2233408384, Redpoint::EOS::Tests::Friends)
{
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

class REDPOINTEOSTESTS_API FVerifyFriendIsNotPresentChainedTask : public FChainedTask
{
private:
    int SourceInstanceIndex;
    int TargetInstanceIndex;

public:
    FVerifyFriendIsNotPresentChainedTask(int InSourceInstanceIndex, int InTargetInstanceIndex);
    UE_NONCOPYABLE(FVerifyFriendIsNotPresentChainedTask);
    virtual ~FVerifyFriendIsNotPresentChainedTask() override = default;

    virtual void ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone) override;
};

}

namespace Redpoint::EOS::Tests::Friends
{
REDPOINT_EOS_FILE_NS_EXPORT(2233408384, Redpoint::EOS::Tests::Friends, FVerifyFriendIsNotPresentChainedTask)
}

REDPOINT_EOS_CODE_GUARD_END()