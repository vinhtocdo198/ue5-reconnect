// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSTests/Friends/VerifyFriendIsNotPresentChainedTask.h"

#include "Interfaces/OnlineFriendsInterface.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2233408384, Redpoint::EOS::Tests::Friends)
{

FVerifyFriendIsNotPresentChainedTask::FVerifyFriendIsNotPresentChainedTask(
    int InSourceInstanceIndex,
    int InTargetInstanceIndex)
    : SourceInstanceIndex(InSourceInstanceIndex)
    , TargetInstanceIndex(InTargetInstanceIndex)
{
}

void FVerifyFriendIsNotPresentChainedTask::ExecuteAsync(
    const FChainedTaskContextRef &Context,
    const FOnComplete &OnDone)
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

    // Make sure the friend is not in there.
    bool bFound = false;
    for (const auto &Friend : Friends)
    {
        if (*Friend->GetUserId() == Context->User(this->TargetInstanceIndex))
        {
            bFound = true;
        }
    }
    Context->Assert().TestFalse("Expected friend not to be present on friends list", bFound);
    OnDone.ExecuteIfBound(true);
}

}

REDPOINT_EOS_CODE_GUARD_END()