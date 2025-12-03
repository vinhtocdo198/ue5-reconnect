// Copyright June Rhodes. All Rights Reserved.

#include "Interfaces/OnlineFriendsInterface.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTests/Friends/AcceptFriendInviteChainedTask.h"
#include "RedpointEOSTests/Friends/DeleteFriendInviteChainedTask.h"
#include "RedpointEOSTests/Friends/SendFriendInviteChainedTask.h"
#include "RedpointEOSTests/Friends/VerifyFriendIsNotPresentChainedTask.h"
#include "RedpointEOSTests/Friends/VerifyFriendIsPresentChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskExecutor.h"
#include "TestHelpers.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1081547041, Redpoint::EOS::Tests::Friends)
{
using namespace ::Redpoint::EOS::Tests::Friends;
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_Friends_DeleteFriend,
    "Redpoint.EOS.Friends.DeleteFriend",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_Friends_DeleteFriend::RunAsyncTest(const TFunction<void()> &OnDone)
{
    TSharedRef<FChainedTaskExecutor> Executor =
        MakeShareable(new FChainedTaskExecutor(this, OnDone, TArray<int32>{1, 1}));

    // Send the invite to the target user.
    Executor->Then<FSendFriendInviteChainedTask>(0, 1);

    // Make sure the friends lists are in expected state.
    Executor->Then<FVerifyFriendIsPresentChainedTask>(0, 1, EInviteStatus::PendingOutbound);
    Executor->Then<FVerifyFriendIsPresentChainedTask>(1, 0, EInviteStatus::PendingInbound);

    // Accept the invite as the target user.
    Executor->Then<FAcceptFriendInviteChainedTask>(1, 0);

    // Make sure that user 1 appears as a friend on user 0's friends list.
    Executor->Then<FVerifyFriendIsPresentChainedTask>(0, 1, EInviteStatus::Accepted);
    Executor->Then<FVerifyFriendIsPresentChainedTask>(1, 0, EInviteStatus::Accepted);

    // Delete the target user.
    Executor->Then<FDeleteFriendInviteChainedTask>(0, 1);

    // Make sure the friends lists are in expected state.
    Executor->Then<FVerifyFriendIsNotPresentChainedTask>(0, 1);
    Executor->Then<FVerifyFriendIsNotPresentChainedTask>(1, 0);

    // Run the test.
    Executor->Execute();
}

}

REDPOINT_EOS_CODE_GUARD_END()
