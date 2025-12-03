// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSTests/Friends/SendFriendInviteChainedTask.h"

#include "Interfaces/OnlineFriendsInterface.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3686893667, Redpoint::EOS::Tests::Friends)
{

FSendFriendInviteChainedTask::FSendFriendInviteChainedTask(int InSenderInstanceId, int InTargetInstanceId)
    : SenderInstanceId(InSenderInstanceId)
    , TargetInstanceId(InTargetInstanceId)
    , Handle()
    , bSendComplete()
    , bSendReceivedComplete()
    , bFiredCallback()
{
}

void FSendFriendInviteChainedTask::ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone)
{
    this->Handle = Context->Instance(this->TargetInstanceId)
                       .GetFriendsInterface()
                       ->AddOnInviteReceivedDelegate_Handle(FOnInviteReceivedDelegate::CreateSP(
                           this,
                           &FSendFriendInviteChainedTask::OnInviteReceived,
                           Context,
                           OnDone));

    Context->Instance(this->SenderInstanceId)
        .GetFriendsInterface()
        ->SendInvite(
            0,
            Context->User(this->TargetInstanceId),
            TEXT(""),
            FOnSendInviteComplete::CreateSP(this, &FSendFriendInviteChainedTask::OnSendInvite, Context, OnDone));
}

void FSendFriendInviteChainedTask::OnInviteReceived(
    const FUniqueNetId &UserId,
    const FUniqueNetId &FriendId,
    FChainedTaskContextRef Context,
    FOnComplete OnDone)
{
    if (UserId == Context->User(this->TargetInstanceId) && FriendId == Context->User(this->SenderInstanceId))
    {
        this->bSendReceivedComplete = true;
        this->CheckForDone(Context, OnDone);
    }
}

void FSendFriendInviteChainedTask::OnSendInvite(
    int32 LocalUserNum,
    bool bWasSuccessful,
    const FUniqueNetId &FriendId,
    const FString &ListName,
    const FString &ErrorStr,
    FChainedTaskContextRef Context,
    FOnComplete OnDone)
{
    Context->Assert().TestTrue(TEXT("Send invite was successful"), bWasSuccessful);

    this->bSendComplete = true;
    this->CheckForDone(Context, OnDone);
}

void FSendFriendInviteChainedTask::CheckForDone(FChainedTaskContextRef Context, FOnComplete OnDone)
{
    if (this->bSendReceivedComplete && this->bSendComplete)
    {
        if (!this->bFiredCallback)
        {
            Context->Instance(this->TargetInstanceId)
                .GetFriendsInterface()
                ->ClearOnInviteReceivedDelegate_Handle(this->Handle);
            this->bFiredCallback = true;
            OnDone.ExecuteIfBound(true);
        }
    }
}

}

REDPOINT_EOS_CODE_GUARD_END()