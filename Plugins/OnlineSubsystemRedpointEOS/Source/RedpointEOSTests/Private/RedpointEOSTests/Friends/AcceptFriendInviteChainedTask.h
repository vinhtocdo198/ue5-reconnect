// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/ChainedTask.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(252149122, Redpoint::EOS::Tests::Friends)
{
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

class REDPOINTEOSTESTS_API FAcceptFriendInviteChainedTask : public FChainedTask,
                                                            public TSharedFromThis<FAcceptFriendInviteChainedTask>
{
private:
    int ReceiverInstanceId;
    int SenderInstanceId;
    FDelegateHandle Handle;
    bool bAcceptComplete;
    bool bAcceptReceivedComplete;
    bool bFiredCallback;

public:
    FAcceptFriendInviteChainedTask(int InReceiverInstanceId, int InSenderInstanceId);
    UE_NONCOPYABLE(FAcceptFriendInviteChainedTask);
    virtual ~FAcceptFriendInviteChainedTask() override = default;

    virtual void ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone) override;

private:
    void OnInviteAccepted(
        const FUniqueNetId &UserId,
        const FUniqueNetId &FriendId,
        FChainedTaskContextRef Context,
        FOnComplete OnDone);
    void OnAcceptInvite(
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
REDPOINT_EOS_FILE_NS_EXPORT(252149122, Redpoint::EOS::Tests::Friends, FAcceptFriendInviteChainedTask)
}

REDPOINT_EOS_CODE_GUARD_END()