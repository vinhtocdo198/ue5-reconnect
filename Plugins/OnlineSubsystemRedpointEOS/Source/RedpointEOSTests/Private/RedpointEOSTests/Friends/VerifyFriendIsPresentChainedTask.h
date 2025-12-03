// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/ChainedTask.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(359464654, Redpoint::EOS::Tests::Friends)
{
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

class REDPOINTEOSTESTS_API FVerifyFriendIsPresentChainedTask : public FChainedTask
{
private:
    int SourceInstanceIndex;
    int TargetInstanceIndex;
    EInviteStatus::Type InviteStatus;

public:
    FVerifyFriendIsPresentChainedTask(
        int InSourceInstanceIndex,
        int InTargetInstanceIndex,
        EInviteStatus::Type InInviteStatus);
    UE_NONCOPYABLE(FVerifyFriendIsPresentChainedTask);
    virtual ~FVerifyFriendIsPresentChainedTask() override = default;

    virtual void ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone) override;
};

}

namespace Redpoint::EOS::Tests::Friends
{
REDPOINT_EOS_FILE_NS_EXPORT(359464654, Redpoint::EOS::Tests::Friends, FVerifyFriendIsPresentChainedTask)
}

REDPOINT_EOS_CODE_GUARD_END()