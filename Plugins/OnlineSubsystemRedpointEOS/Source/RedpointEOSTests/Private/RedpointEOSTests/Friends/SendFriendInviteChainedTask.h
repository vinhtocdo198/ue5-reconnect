// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/ChainedTask.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3686893667, Redpoint::EOS::Tests::Friends)
{
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

class REDPOINTEOSTESTS_API FSendFriendInviteChainedTask : public FChainedTask,
                                                          public TSharedFromThis<FSendFriendInviteChainedTask>
{
private:
    int SenderInstanceId;
    int TargetInstanceId;
    FDelegateHandle Handle;
    bool bSendComplete;
    bool bSendReceivedComplete;
    bool bFiredCallback;

public:
    FSendFriendInviteChainedTask(int InSenderInstanceId, int InTargetInstanceId);
    UE_NONCOPYABLE(FSendFriendInviteChainedTask);
    virtual ~FSendFriendInviteChainedTask() override = default;

    virtual void ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone) override;

private:
    void OnInviteReceived(
        const FUniqueNetId &UserId,
        const FUniqueNetId &FriendId,
        FChainedTaskContextRef Context,
        FOnComplete OnDone);
    void OnSendInvite(
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
REDPOINT_EOS_FILE_NS_EXPORT(3686893667, Redpoint::EOS::Tests::Friends, FSendFriendInviteChainedTask)
}

REDPOINT_EOS_CODE_GUARD_END()