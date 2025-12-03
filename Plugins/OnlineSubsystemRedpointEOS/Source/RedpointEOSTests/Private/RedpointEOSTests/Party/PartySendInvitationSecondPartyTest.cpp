// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTests/Party/CreatePartyChainedTask.h"
#include "RedpointEOSTests/Party/JoinPartyViaInviteChainedTask.h"
#include "RedpointEOSTests/Party/LeavePartyChainedTask.h"
#include "RedpointEOSTests/Party/RejectAllPendingInvitesChainedTask.h"
#include "RedpointEOSTests/Party/SendAndReceiveInvitationChainedTask.h"
#include "RedpointEOSTests/Party/VerifyPartyMembershipChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/LambdaChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/RegisterEventChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/UnregisterEventChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/WaitForEventChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskExecutor.h"
#include "TestHelpers.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(239696798, Redpoint::EOS::Tests::Party)
{
using namespace ::Redpoint::EOS::Tests::Party;

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_Party_SendInvitationSecondParty,
    "Redpoint.EOS.Party.SendInvitationSecondParty",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_Party_SendInvitationSecondParty::RunAsyncTest(const TFunction<void()> &OnDone)
{
    using namespace ::Redpoint::EOS::Tests::TestUtilities;
    using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

    TSharedRef<FChainedTaskExecutor> Executor =
        MakeShareable(new FChainedTaskExecutor(this, OnDone, TArray<int32>{1, 1}));

    // Create shared pointers to hold data.
    TSharedRef<TSharedPtr<const FOnlinePartyId>> StoredPartyId1 = MakeShared<TSharedPtr<const FOnlinePartyId>>();
    TSharedRef<TSharedPtr<const FOnlinePartyId>> StoredPartyId2 = MakeShared<TSharedPtr<const FOnlinePartyId>>();
    TSharedRef<TSharedPtr<const IOnlinePartyJoinInfo>> StoredPartyJoinInfo1 =
        MakeShared<TSharedPtr<const IOnlinePartyJoinInfo>>();
    TSharedRef<TSharedPtr<const IOnlinePartyJoinInfo>> StoredPartyJoinInfo2 =
        MakeShared<TSharedPtr<const IOnlinePartyJoinInfo>>();

    // Create multiple parties as the first user.
    Executor->Then<FCreatePartyChainedTask>(0, StoredPartyId1, FOnlinePartyTypeId(100));
    Executor->Then<FVerifyPartyMembershipChainedTask>(0, FVerifyPartyMembershipChainedTask::FInitList{StoredPartyId1});
    Executor->Then<FCreatePartyChainedTask>(0, StoredPartyId2, FOnlinePartyTypeId(200));
    Executor->Then<FVerifyPartyMembershipChainedTask>(
        0,
        FVerifyPartyMembershipChainedTask::FInitList{StoredPartyId1, StoredPartyId2});

    // Clear out any pending invitations so that we will only see the ones we're about to send.
    Executor->Then<FRejectAllPendingInvitesChainedTask>(0);
    Executor->Then<FRejectAllPendingInvitesChainedTask>(1);

    // Make sure we can send and receive an invitation.
    Executor->Then<FSendAndReceiveInvitationChainedTask>(0, 1, StoredPartyId1, StoredPartyJoinInfo1);
    Executor->Then<FSendAndReceiveInvitationChainedTask>(0, 1, StoredPartyId2, StoredPartyJoinInfo2);

    // Join both parties using the invites.
    Executor->Then<FVerifyPartyMembershipChainedTask>(1, FVerifyPartyMembershipChainedTask::FInitList{});
    Executor->Then<FJoinPartyViaInviteChainedTask>(1, StoredPartyJoinInfo1);
    Executor->Then<FVerifyPartyMembershipChainedTask>(1, FVerifyPartyMembershipChainedTask::FInitList{StoredPartyId1});
    Executor->Then<FJoinPartyViaInviteChainedTask>(1, StoredPartyJoinInfo2);
    Executor->Then<FVerifyPartyMembershipChainedTask>(
        1,
        FVerifyPartyMembershipChainedTask::FInitList{StoredPartyId1, StoredPartyId2});

    // Double check party state.
    Executor->Then<FVerifyPartyMembershipChainedTask>(
        0,
        FVerifyPartyMembershipChainedTask::FInitList{StoredPartyId1, StoredPartyId2});
    Executor->Then<FVerifyPartyMembershipChainedTask>(
        1,
        FVerifyPartyMembershipChainedTask::FInitList{StoredPartyId1, StoredPartyId2});

    // Make the host leave both parties.
    Executor->Then<FLeavePartyChainedTask>(0, StoredPartyId1);
    Executor->Then<FVerifyPartyMembershipChainedTask>(0, FVerifyPartyMembershipChainedTask::FInitList{StoredPartyId2});
    Executor->Then<FVerifyPartyMembershipChainedTask>(
        1,
        FVerifyPartyMembershipChainedTask::FInitList{StoredPartyId1, StoredPartyId2});
    Executor->Then<FLeavePartyChainedTask>(0, StoredPartyId2);
    Executor->Then<FVerifyPartyMembershipChainedTask>(0, FVerifyPartyMembershipChainedTask::FInitList{});
    Executor->Then<FVerifyPartyMembershipChainedTask>(
        1,
        FVerifyPartyMembershipChainedTask::FInitList{StoredPartyId1, StoredPartyId2});

    // Make the client leave both parties.
    Executor->Then<FLeavePartyChainedTask>(1, StoredPartyId1);
    Executor->Then<FVerifyPartyMembershipChainedTask>(0, FVerifyPartyMembershipChainedTask::FInitList{});
    Executor->Then<FVerifyPartyMembershipChainedTask>(1, FVerifyPartyMembershipChainedTask::FInitList{StoredPartyId2});
    Executor->Then<FLeavePartyChainedTask>(1, StoredPartyId2);
    Executor->Then<FVerifyPartyMembershipChainedTask>(0, FVerifyPartyMembershipChainedTask::FInitList{});
    Executor->Then<FVerifyPartyMembershipChainedTask>(1, FVerifyPartyMembershipChainedTask::FInitList{});

    // Run the test.
    Executor->Execute();
}

}

REDPOINT_EOS_CODE_GUARD_END()