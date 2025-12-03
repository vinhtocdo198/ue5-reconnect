// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSTests/Friends/RejectFriendInviteChainedTask.h"

#include "Interfaces/OnlineFriendsInterface.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1887202375, Redpoint::EOS::Tests::Friends)
{

FRejectFriendInviteChainedTask::FRejectFriendInviteChainedTask(int InReceiverInstanceId, int InSenderInstanceId)
    : ReceiverInstanceId(InReceiverInstanceId)
    , SenderInstanceId(InSenderInstanceId)
    , SenderHandle()
    , ReceiverHandle()
    , bRejectComplete()
    , bRejectReceivedComplete()
    , bFiredCallback()
{
}

void FRejectFriendInviteChainedTask::ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone)
{
    this->SenderHandle = Context->Instance(this->SenderInstanceId)
                             .GetFriendsInterface()
                             ->AddOnInviteRejectedDelegate_Handle(FOnInviteRejectedDelegate::CreateSP(
                                 this,
                                 &FRejectFriendInviteChainedTask::OnInviteRejected,
                                 Context,
                                 OnDone));

    this->ReceiverHandle = Context->Instance(this->ReceiverInstanceId)
                               .GetFriendsInterface()
                               ->AddOnRejectInviteCompleteDelegate_Handle(
                                   0,
                                   FOnRejectInviteCompleteDelegate::CreateSP(
                                       this,
                                       &FRejectFriendInviteChainedTask::OnRejectInvite,
                                       Context,
                                       OnDone));

    if (!Context->Assert().TestTrue(
            TEXT("Expected RejectInvite to return true"),
            Context->Instance(this->ReceiverInstanceId)
                .GetFriendsInterface()
                ->RejectInvite(0, Context->User(this->SenderInstanceId), TEXT(""))))
    {
        OnDone.ExecuteIfBound(false);
    }
}

void FRejectFriendInviteChainedTask::OnInviteRejected(
    const FUniqueNetId &UserId,
    const FUniqueNetId &FriendId,
    FChainedTaskContextRef Context,
    FOnComplete OnDone)
{
    if (UserId == Context->User(this->SenderInstanceId) && FriendId == Context->User(this->ReceiverInstanceId))
    {
        this->bRejectReceivedComplete = true;
        this->CheckForDone(Context, OnDone);
    }
}

void FRejectFriendInviteChainedTask::OnRejectInvite(
    int32 LocalUserNum,
    bool bWasSuccessful,
    const FUniqueNetId &FriendId,
    const FString &ListName,
    const FString &ErrorStr,
    FChainedTaskContextRef Context,
    FOnComplete OnDone)
{
    Context->Assert().TestTrue(TEXT("Reject invite was successful"), bWasSuccessful);

    this->bRejectComplete = true;
    this->CheckForDone(Context, OnDone);
}

void FRejectFriendInviteChainedTask::CheckForDone(FChainedTaskContextRef Context, FOnComplete OnDone)
{
    if (this->bRejectReceivedComplete && this->bRejectComplete)
    {
        if (!this->bFiredCallback)
        {
            Context->Instance(this->SenderInstanceId)
                .GetFriendsInterface()
                ->ClearOnInviteReceivedDelegate_Handle(this->SenderHandle);
            Context->Instance(this->ReceiverInstanceId)
                .GetFriendsInterface()
                ->ClearOnRejectInviteCompleteDelegate_Handle(0, this->ReceiverHandle);
            this->bFiredCallback = true;
            OnDone.ExecuteIfBound(true);
        }
    }
}

}

REDPOINT_EOS_CODE_GUARD_END()