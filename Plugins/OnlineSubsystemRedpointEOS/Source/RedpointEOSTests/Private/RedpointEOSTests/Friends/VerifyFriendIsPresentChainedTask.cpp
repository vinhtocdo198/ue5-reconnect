// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSTests/Friends/VerifyFriendIsPresentChainedTask.h"

#include "Interfaces/OnlineFriendsInterface.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(359464654, Redpoint::EOS::Tests::Friends)
{

FVerifyFriendIsPresentChainedTask::FVerifyFriendIsPresentChainedTask(
    int InSourceInstanceIndex,
    int InTargetInstanceIndex,
    EInviteStatus::Type InInviteStatus)
    : SourceInstanceIndex(InSourceInstanceIndex)
    , TargetInstanceIndex(InTargetInstanceIndex)
    , InviteStatus(InInviteStatus)
{
}

void FVerifyFriendIsPresentChainedTask::ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone)
{
    // Get all of the user's friends.
    TArray<TSharedRef<FOnlineFriend>> Friends;
    if (!Context->Assert().TestTrue(
            TEXT("Expected GetFriendsList to succeed"),
            Context->Instance(this->SourceInstanceIndex).GetFriendsInterface()->GetFriendsList(0, TEXT(""), Friends)))
    {
        OnDone.ExecuteIfBound(false);
        return;
    }

    // Make sure the friend is in there.
    bool bFound = false;
    for (const auto &Friend : Friends)
    {
        if (*Friend->GetUserId() == Context->User(this->TargetInstanceIndex) &&
            Friend->GetInviteStatus() == this->InviteStatus)
        {
            bFound = true;
        }
    }
    Context->Assert().TestTrue("Expected friend to be present on friends list", bFound);
    OnDone.ExecuteIfBound(true);
}

}

REDPOINT_EOS_CODE_GUARD_END()