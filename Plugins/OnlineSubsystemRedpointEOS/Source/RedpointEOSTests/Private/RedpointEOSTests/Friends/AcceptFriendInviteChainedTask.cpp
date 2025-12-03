// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSTests/Friends/AcceptFriendInviteChainedTask.h"

#include "Interfaces/OnlineFriendsInterface.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(252149122, Redpoint::EOS::Tests::Friends)
{

FAcceptFriendInviteChainedTask::FAcceptFriendInviteChainedTask(int InReceiverInstanceId, int InSenderInstanceId)
    : ReceiverInstanceId(InReceiverInstanceId)
    , SenderInstanceId(InSenderInstanceId)
    , Handle()
    , bAcceptComplete()
    , bAcceptReceivedComplete()
    , bFiredCallback()
{
}

void FAcceptFriendInviteChainedTask::ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone)
{
    this->Handle = Context->Instance(this->SenderInstanceId)
                       .GetFriendsInterface()
                       ->AddOnInviteAcceptedDelegate_Handle(FOnInviteAcceptedDelegate::CreateSP(
                           this,
                           &FAcceptFriendInviteChainedTask::OnInviteAccepted,
                           Context,
                           OnDone));

    Context->Instance(this->ReceiverInstanceId)
        .GetFriendsInterface()
        ->AcceptInvite(
            0,
            Context->User(this->SenderInstanceId),
            TEXT(""),
            FOnAcceptInviteComplete::CreateSP(this, &FAcceptFriendInviteChainedTask::OnAcceptInvite, Context, OnDone));
}

void FAcceptFriendInviteChainedTask::OnInviteAccepted(
    const FUniqueNetId &UserId,
    const FUniqueNetId &FriendId,
    FChainedTaskContextRef Context,
    FOnComplete OnDone)
{
    if (UserId == Context->User(this->SenderInstanceId) && FriendId == Context->User(this->ReceiverInstanceId))
    {
        this->bAcceptReceivedComplete = true;
        this->CheckForDone(Context, OnDone);
    }
}

void FAcceptFriendInviteChainedTask::OnAcceptInvite(
    int32 LocalUserNum,
    bool bWasSuccessful,
    const FUniqueNetId &FriendId,
    const FString &ListName,
    const FString &ErrorStr,
    FChainedTaskContextRef Context,
    FOnComplete OnDone)
{
    Context->Assert().TestTrue(TEXT("Accept invite was successful"), bWasSuccessful);

    this->bAcceptComplete = true;
    this->CheckForDone(Context, OnDone);
}

void FAcceptFriendInviteChainedTask::CheckForDone(FChainedTaskContextRef Context, FOnComplete OnDone)
{
    if (this->bAcceptReceivedComplete && this->bAcceptComplete)
    {
        if (!this->bFiredCallback)
        {
            Context->Instance(this->SenderInstanceId)
                .GetFriendsInterface()
                ->ClearOnInviteReceivedDelegate_Handle(this->Handle);
            this->bFiredCallback = true;
            OnDone.ExecuteIfBound(true);
        }
    }
}

}

REDPOINT_EOS_CODE_GUARD_END()