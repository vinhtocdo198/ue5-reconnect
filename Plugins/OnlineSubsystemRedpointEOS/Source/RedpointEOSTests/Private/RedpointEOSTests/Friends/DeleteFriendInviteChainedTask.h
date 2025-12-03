// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/ChainedTask.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(54748632, Redpoint::EOS::Tests::Friends)
{
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

class REDPOINTEOSTESTS_API FDeleteFriendInviteChainedTask : public FChainedTask,
                                                            public TSharedFromThis<FDeleteFriendInviteChainedTask>
{
private:
    int SenderInstanceId;
    int TargetInstanceId;
    FDelegateHandle TargetHandle;
    FDelegateHandle SenderHandle;
    bool bDeleteComplete;
    bool bRemovedComplete;
    bool bFiredCallback;

public:
    FDeleteFriendInviteChainedTask(int InSenderInstanceId, int InTargetInstanceId);
    UE_NONCOPYABLE(FDeleteFriendInviteChainedTask);
    virtual ~FDeleteFriendInviteChainedTask() override = default;

    virtual void ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone) override;

private:
    void OnFriendRemoved(
        const FUniqueNetId &UserId,
        const FUniqueNetId &FriendId,
        FChainedTaskContextRef Context,
        FOnComplete OnDone);
    void OnDeleteFriendComplete(
        int32 LocalUserNum,
        bool bWasSuccessful,
        const FUniqueNetId &FriendId,
        const FString &ListName,
        const FString &ErrorStr,
        FChainedTaskContextRef Context,
        FOnComplete OnDone);

    void CheckForDone(FChainedTaskContextRef Context, FOnComplete OnDone);
};

}

namespace Redpoint::EOS::Tests::Friends
{
REDPOINT_EOS_FILE_NS_EXPORT(54748632, Redpoint::EOS::Tests::Friends, FDeleteFriendInviteChainedTask)
}

REDPOINT_EOS_CODE_GUARD_END()