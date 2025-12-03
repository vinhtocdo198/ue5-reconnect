// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSTests/Friends/DeleteFriendInviteChainedTask.h"

#include "Interfaces/OnlineFriendsInterface.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(54748632, Redpoint::EOS::Tests::Friends)
{

FDeleteFriendInviteChainedTask::FDeleteFriendInviteChainedTask(int InSenderInstanceId, int InTargetInstanceId)
    : SenderInstanceId(InSenderInstanceId)
    , TargetInstanceId(InTargetInstanceId)
    , TargetHandle()
    , SenderHandle()
    , bDeleteComplete()
    , bRemovedComplete()
    , bFiredCallback()
{
}

void FDeleteFriendInviteChainedTask::ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone)
{
    this->TargetHandle = Context->Instance(this->TargetInstanceId)
                             .GetFriendsInterface()
                             ->AddOnFriendRemovedDelegate_Handle(FOnFriendRemovedDelegate::CreateSP(
                                 this,
                                 &FDeleteFriendInviteChainedTask::OnFriendRemoved,
                                 Context,
                                 OnDone));

    this->SenderHandle = Context->Instance(this->SenderInstanceId)
                             .GetFriendsInterface()
                             ->AddOnDeleteFriendCompleteDelegate_Handle(
                                 0,
                                 FOnDeleteFriendCompleteDelegate::CreateSP(
                                     this,
                                     &FDeleteFriendInviteChainedTask::OnDeleteFriendComplete,
                                     Context,
                                     OnDone));

    Context->Assert().TestTrue(
        TEXT("DeleteFriend should start"),
        Context->Instance(this->SenderInstanceId)
            .GetFriendsInterface()
            ->DeleteFriend(0, Context->User(this->TargetInstanceId), TEXT("")));
}

void FDeleteFriendInviteChainedTask::OnFriendRemoved(
    const FUniqueNetId &UserId,
    const FUniqueNetId &FriendId,
    FChainedTaskContextRef Context,
    FOnComplete OnDone)
{
    if (UserId == Context->User(this->TargetInstanceId) && FriendId == Context->User(this->SenderInstanceId))
    {
        this->bRemovedComplete = true;
        this->CheckForDone(Context, OnDone);
    }
}

void FDeleteFriendInviteChainedTask::OnDeleteFriendComplete(
    int32 LocalUserNum,
    bool bWasSuccessful,
    const FUniqueNetId &FriendId,
    const FString &ListName,
    const FString &ErrorStr,
    FChainedTaskContextRef Context,
    FOnComplete OnDone)
{
    Context->Assert().TestTrue(TEXT("Delete friend was successful"), bWasSuccessful);

    Context->Instance(this->SenderInstanceId)
        .GetFriendsInterface()
        ->ClearOnDeleteFriendCompleteDelegate_Handle(0, this->SenderHandle);

    this->bDeleteComplete = true;
    this->CheckForDone(Context, OnDone);
}

void FDeleteFriendInviteChainedTask::CheckForDone(FChainedTaskContextRef Context, FOnComplete OnDone)
{
    if (this->bRemovedComplete && this->bDeleteComplete)
    {
        if (!this->bFiredCallback)
        {
            Context->Instance(this->TargetInstanceId)
                .GetFriendsInterface()
                ->ClearOnFriendRemovedDelegate_Handle(this->TargetHandle);
            Context->Instance(this->SenderInstanceId)
                .GetFriendsInterface()
                ->ClearOnDeleteFriendCompleteDelegate_Handle(0, this->SenderHandle);
            this->bFiredCallback = true;
            OnDone.ExecuteIfBound(true);
        }
    }
}

}

REDPOINT_EOS_CODE_GUARD_END()