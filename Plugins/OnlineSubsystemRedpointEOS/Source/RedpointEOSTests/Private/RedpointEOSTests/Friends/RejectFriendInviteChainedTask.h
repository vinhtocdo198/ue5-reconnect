// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/ChainedTask.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1887202375, Redpoint::EOS::Tests::Friends)
{
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

class REDPOINTEOSTESTS_API FRejectFriendInviteChainedTask : public FChainedTask,
                                                            public TSharedFromThis<FRejectFriendInviteChainedTask>
{
private:
    int ReceiverInstanceId;
    int SenderInstanceId;
    FDelegateHandle SenderHandle;
    FDelegateHandle ReceiverHandle;
    bool bRejectComplete;
    bool bRejectReceivedComplete;
    bool bFiredCallback;

public:
    FRejectFriendInviteChainedTask(int InReceiverInstanceId, int InSenderInstanceId);
    UE_NONCOPYABLE(FRejectFriendInviteChainedTask);
    virtual ~FRejectFriendInviteChainedTask() override = default;

    virtual void ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone) override;

private:
    void OnInviteRejected(
        const FUniqueNetId &UserId,
        const FUniqueNetId &FriendId,
        FChainedTaskContextRef Context,
        FOnComplete OnDone);
    void OnRejectInvite(
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
REDPOINT_EOS_FILE_NS_EXPORT(1887202375, Redpoint::EOS::Tests::Friends, FRejectFriendInviteChainedTask)
}

REDPOINT_EOS_CODE_GUARD_END()